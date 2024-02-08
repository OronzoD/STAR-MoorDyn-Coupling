package macro;

import java.util.*;

import star.common.*;
import star.base.neo.*;
import star.sixdof.*;

public class initialize_MoorDyn extends StarMacro {

  public void execute() {
    execute0();
  }

  private void execute0() {

    Simulation simulation_0 = getActiveSimulation();
	//GET DIR NAME
	String dir = simulation_0.getSessionDir(); 

	//LOAD LIBRARY
    UserLibrary userLibrary_0 = simulation_0.getUserFunctionManager().createUserLibrary(resolvePath(dir+"//test.so")); 
    
	//GET DFBI BODY
    ContinuumBody continuumBody_0 = ((ContinuumBody) simulation_0.get(star.sixdof.BodyManager.class).getObject("Body 1"));
	
	//GET USER DEFINED COUPLING
	UserDefinedCoupling userDefinedCoupling_0 = ((UserDefinedCoupling) simulation_0.get(SixDofBodyCouplingManager.class).getObject("Mooring"));
	
	// LINK MOORING FORCES
    ForceProfile forceProfile_0 = userDefinedCoupling_0.getForceProfile();
	forceProfile_0.setMethod(UserVectorProfileMethod.class);
    UserFunction userFunction_0 = userLibrary_0.getFunction("MooringForces");
    forceProfile_0.getMethod(UserVectorProfileMethod.class).setFunction(userFunction_0);
	
	//LINK MOORING MOMENTS	
	MomentProfile momentProfile_0 = userDefinedCoupling_0.getMomentProfile();	
	momentProfile_0.setMethod(UserVectorProfileMethod.class);
	UserFunction userFunction_1 = userLibrary_0.getFunction("MooringMoments");	
	momentProfile_0.getMethod(UserVectorProfileMethod.class).setFunction(userFunction_1);

 // SAVE 
    String sep = System.getProperty("file.separator"); //get the right separator for your operative system    
    String filename = simulation_0.getPresentationName(); //get the name of the current sim file       
    simulation_0.saveState(dir + sep + filename + "@moordyn.sim"); //save the current sim file as name_of_the_file@mod.sim

  }
}
