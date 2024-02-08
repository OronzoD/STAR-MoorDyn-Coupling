#include "../Coupling/uclib.h"
#include "../MoorDyn-master/MoorDyn.h"
#include <unistd.h>

const int DOF = 6;
// SCALE=1 if no symmetry. SCALE=2 if there is symmetry along y=0. REMEMBER TO CHANGE IT!!!!!
const int SCALE = 1;			
double timestep = 0;
int lastTimeLevel = -1;
int iter=0;
//Forces and Moments are saved to be used in all inner iteration of Star CCm+ and evaluated only at the end of the time step
double cache_f1=0,cache_f2=0,cache_f3=0,cache_m1=0,cache_m2=0,cache_m3=0,counter=1; 
// Coordinate of CoG from Laboratory Coordinate System.  REMEMBER TO CHANGE IT!!!!!
double Xcg=0,Ycg=0,Zcg=0; 
// Rotation of CoG in respect to default configuration referred to lines.txt.  REMEMBER TO CHANGE IT!!!!!
//These angle are summed to the rotation of the body read from Star becouse the reports starts always from 0. Useful in Free-Decay tests
double Phi=0, Theta=0, Psi=0;
// Inner Iteration in Star. REMEMBER TO CHANGE IT!!!!!
int II=8;

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
	double x[6] = { Xcg,Ycg,Zcg,Phi,Teta,Psi};
	double xd[6] = { 0, 0, 0, 0, 0, 0 };

	timestep=0;
	
	// MoorDyn Initialization
	LinesInit(x,xd);

	char cwd[1024];
	chdir("/path/to/change/directory/to");
	getcwd(cwd, sizeof(cwd));

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

