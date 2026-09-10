#include "../Coupling/uclib.h"
#include "../MoorDyn-1.01.02/MoorDyn.h"
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

const int DOF = 6;
// SCALE=1 if no symmetry. SCALE=2 if there is symmetry along y=0. REMEMBER TO CHANGE IT!!!!!
const int SCALE = 1;			
int lastTimeLevel = -1;
int iter=0;
//Forces and Moments are saved to be used in all inner iteration of Star CCm+ and evaluated only at the end of the time step
double cache_f1=0,cache_f2=0,cache_f3=0,cache_m1=0,cache_m2=0,cache_m3=0,counter=1; 
// Coordinate of CoG from Laboratory Coordinate System.  REMEMBER TO CHANGE IT!!!!!
double Xcg=0,Ycg=0,Zcg=-1; 
// Rotation of CoG in respect to default configuration referred to lines.txt.  REMEMBER TO CHANGE IT!!!!!
//These angle are summed to the rotation of the body read from Star becouse the reports starts always from 0. Useful in Free-Decay tests
double Phi=0, Theta=0, Psi=0;
// Inner Iteration in Star. REMEMBER TO CHANGE IT!!!!!
int II=8;

#ifdef _WIN32
// MoorDyn v1 opens Mooring/lines.txt using a relative path. STAR-CCM+ does
// not guarantee that the process working directory is the simulation folder,
// so use the directory containing this loaded DLL.
static void SetWorkingDirectoryToLibraryFolder()
{
	HMODULE module = nullptr;
	char modulePath[MAX_PATH] = {0};
	if (GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCSTR>(&SetWorkingDirectoryToLibraryFolder),
			&module) &&
		GetModuleFileNameA(module, modulePath, MAX_PATH) > 0)
	{
		char *lastSeparator = strrchr(modulePath, '\\');
		if (lastSeparator != nullptr)
		{
			*lastSeparator = '\0';
			_chdir(modulePath);
		}
	}
}
#else
// Linux equivalent of the Windows implementation above. dladdr() returns the
// path of the shared object containing the supplied function address.
static void SetWorkingDirectoryToLibraryFolder()
{
	Dl_info libraryInfo = {0};
	if (dladdr(reinterpret_cast<void *>(&SetWorkingDirectoryToLibraryFolder),
			   &libraryInfo) != 0 && libraryInfo.dli_fname != nullptr)
	{
		char libraryPath[4096] = {0};
		std::strncpy(libraryPath, libraryInfo.dli_fname,
					sizeof(libraryPath) - 1);
		char *lastSeparator = strrchr(libraryPath, '/');
		if (lastSeparator != nullptr)
		{
			*lastSeparator = '\0';
			chdir(libraryPath);
		}
	}
}
#endif

extern "C" void MooringMoments(
		Real (*result)[3],       	// Output of the Function, Mooring Forces  
		int size)                	// Needed 					         

	{
	if (size != 2)
		return;

	result[0][0]=cache_m1;
	result[0][1]=cache_m2;
	result[0][2]=cache_m3;
}

extern "C" void MooringForces(
		Real (*result)[3],			// Output of the Function, Mooring Forces
		int size,					// Needed	
		CoordReal (*xf)[3],			// Position of CoG in respect of the Laboratory
		CoordReal (*xdf)[3],		// Velocity of CoG in respect of the Laboratory
		CoordReal (*xfr)[3],		// Rotation of CoG in respect to initial Condition
		CoordReal (*xdfr)[3],		// Angular Velocity of CoG
		Real *TIMELEVEL,		
		Real *TIMESTEP,
		Real *Iteration)
	{
	if (size != 2)
		return;

	result[0][0]=0;
	result[0][1]=0;
	result[0][2]=0;

	double x[DOF] 	= {xf[0][0],xf[0][1],xf[0][2],Phi+xfr[0][0],Theta+xfr[0][1],Psi+xfr[0][2]};
	double xd[DOF] 	= {xdf[0][0],xdf[0][1],xdf[0][2],xdfr[0][0],xdfr[0][1],xdfr[0][2]};

	double Ts[10] = {0,0,0,0,0,0,0,0,0,0};
	double FairTens1[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	double FLines_temp[6] = { 0, 0, 0, 0, 0, 0 };
	double fout[6] = { 0, 0, 0, 0, 0, 0 };	
	double dt = *TIMESTEP;
	double moordyn_t = (*TIMELEVEL) * (*TIMESTEP);
	
	iter=*Iteration;
	if ((iter%II)!=0) {
		
		result[0][0]=cache_f1;
		result[0][1]=cache_f2;
		result[0][2]=cache_f3;
		return;	
	}
	
	if (counter==1) {
		result[0][0]=cache_f1;
		result[0][1]=cache_f2;
		result[0][2]=cache_f3;
		counter=0;
		return;
	}
	
	counter=1;
	
	std::cout << "  Input Kinematic: X(";
	for (int i=0; i<6; i++) {
		std::cout << ((double)x[i])<<", ";
	}
	std::cout << ") \n" << std::flush;
	
	std::cout << "  Input Velocity: Xd(";
	for (int i=0; i<6; i++) {
		std::cout << ((double)xd[i])<<", ";
	}
	std::cout << ") \n" << std::flush;
	
	// MoorDyn function, Generate fout
	LinesCalc(x,xd,fout,&moordyn_t,&dt);  
	
	std::cout << "  bycalc ("<<*TIMELEVEL<<","<<moordyn_t<<") F(";
	for (int i=0; i<3; i++) {
		std::cout << ((double)fout[i]/SCALE)<<", ";
	}
	std::cout<<") M ("; 
	for (int i=3; i<6; i++) {
		std::cout << ((double)fout[i]/SCALE)<<", ";
	}
	std::cout << ") \n" << std::flush;

	// REMEMBER TO CHANGE SCALE FACTOR!!!!
	// Forces are saved for other Inner Iteration of Star
	cache_f1=fout[0]/SCALE;                   
	cache_f2=fout[1]/SCALE;                   
	cache_f3=fout[2]/SCALE;
	// Moments are saved for other Inner Iteration of Star
	cache_m1=fout[3]/SCALE;                             
	cache_m2=fout[4]/SCALE;                             
	cache_m3=fout[5]/SCALE;                            
	// Output forces of Moordyn
	result[0][0]=fout[0]/SCALE;               
	result[0][1]=fout[1]/SCALE;               
	result[0][2]=fout[2]/SCALE;               
}

extern "C" void uclib() {

	LinesClose();
	// Initial Position and Rotation of the CoG
	double x[6] = { Xcg,Ycg,Zcg,Phi,Theta,Psi};
	double xd[6] = { 0, 0, 0, 0, 0, 0 };

	SetWorkingDirectoryToLibraryFolder();

	// This must happen after changing directory because LinesInit() opens
	// Mooring/lines.txt.
	int initResult = LinesInit(x,xd);
	if (initResult != 0)
	{
		std::cerr << "MoorDyn initialization failed (LinesInit returned "
				  << initResult << "). Check that Mooring/lines.txt exists in: ";
		char failedCwd[1024];
		if (getcwd(failedCwd, sizeof(failedCwd)) != nullptr)
			std::cerr << failedCwd;
		else
			std::cerr << "the current working directory";
		std::cerr << std::endl;
		return;
	}

	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) == nullptr) {
		std::snprintf(cwd, sizeof(cwd), "<current directory unavailable>");
	}

	std::cout << "\n\n\n" <<
	"################################################################## \n" <<
	"#### MoorDynStar LOADED SUCCESSFULLY!                         #### \n" <<
	"####                                                          #### \n" <<
	"#### Path lines.txt:	"<<cwd<<"\n"<<
	"####                                                          #### \n" <<
	"################################################################## \n\n\n\n"<<std::flush;
	
	// Definition of Name functions in Star and in the Library
	ucfunc((void*)MooringForces, "PointSetProfile", "MooringForces");
	// Custom Filed Function. It need to be created in Star. Need to have the distance of the CoG from the origin of the Laboratory Coordinate System
	ucarg((void*)MooringForces, "Point", "$$BodyDistance", sizeof(CoordReal[3]));
	// Native Function of Star
	ucarg((void*)MooringForces, "Point", "$$BodyVelocity", sizeof(CoordReal[3]));
	// Custom Filed Function. It need to be created in Star. Need to have the rotation of the CoG
	ucarg((void*)MooringForces, "Point", "$$BodyRotation", sizeof(CoordReal[3]));
	// Native Function of Star
	ucarg((void*)MooringForces, "Point", "$$BodyAngularVelocity", sizeof(CoordReal[3]));
	// Native Function of Star
	ucarg((void*)MooringForces, "Point", "$TimeLevel", sizeof(Real));
	// Native Function of Star
	ucarg((void*)MooringForces, "Point", "$TimeStep", sizeof(Real));
	// Native Function of Star
	ucarg((void*)MooringForces, "Point", "$Iteration", sizeof(Real));
	
	// Definition of Name functions in Star and in the Library
	ucfunc((void*)MooringMoments, "PointSetProfile", "MooringMoments");
}

class MoordynWrap {
};

