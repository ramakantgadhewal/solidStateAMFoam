// Initialise fluid field pointer lists

// Transport model
PtrList<immiscibleIncompressibleTwoPhaseThermalMixture> twoPhaseFluid(fluidRegions.size());

// Flow variables
PtrList<volVectorField> UFluid(fluidRegions.size());
PtrList<surfaceScalarField> phiFluid(fluidRegions.size());
PtrList<volScalarField> pFluid(fluidRegions.size());
PtrList<volScalarField> p_rghFluid(fluidRegions.size());
PtrList<volScalarField> TFluid(fluidRegions.size());

// Gravity terms for pressure calculations
PtrList<uniformDimensionedVectorField> gFluid(fluidRegions.size());
PtrList<uniformDimensionedScalarField> hRefFluid(fluidRegions.size());
PtrList<volScalarField> ghFluid(fluidRegions.size());
PtrList<surfaceScalarField> ghfFluid(fluidRegions.size());

// Weld velocity field for modified reference frame
PtrList<volVectorField> UtFluid(fluidRegions.size());

// Viscosity model variables
PtrList<IOdictionary> transportPropertiesl(fluidRegions.size());
PtrList<volScalarField> muEffl(fluidRegions.size());
PtrList<volScalarField> epsilonDotEql(fluidRegions.size());

// Fluid average physical properties
PtrList<volScalarField> alpha1l(fluidRegions.size());
PtrList<volScalarField> alpha2l(fluidRegions.size());
PtrList<volScalarField> rhoFluid(fluidRegions.size());
PtrList<surfaceScalarField> rhoPhiFluid(fluidRegions.size()); // mass flux using rho & phi
PtrList<volScalarField> rhoCpFluid(fluidRegions.size()); // need to store rho*cp for ddt(rho*cp,T)
PtrList<surfaceScalarField> rhoPhiCpFluid(fluidRegions.size()); // energy flux
PtrList<volScalarField> kFluid(fluidRegions.size());
PtrList<volScalarField> cpFluid(fluidRegions.size());

// Stress and strain-related variables
List<bool> trackStressStrainl(fluidRegions.size());
PtrList<volScalarField> ZFluid(fluidRegions.size());
PtrList<volScalarField> sigmafFluid(fluidRegions.size());
PtrList<volScalarField> epsilonEqFluid(fluidRegions.size());
PtrList<dimensionedScalar> gammaEpsilonFluid(fluidRegions.size());
PtrList<volScalarField> sigmavmFluid(fluidRegions.size());

// Particle tracking variables
List<bool> particleTrackingl(fluidRegions.size());
PtrList<volScalarField> betaFluid(fluidRegions.size());
PtrList<dimensionedScalar> gammaBetaFluid(fluidRegions.size());

// Other necessary lists
PtrList<incompressible::turbulenceModel> turbulenceFluid(fluidRegions.size());
PtrList<radiation::radiationModel> radiationFluid(fluidRegions.size());
List<label> pRefCellFluid(fluidRegions.size(),0);
List<scalar> pRefValueFluid(fluidRegions.size(),0.0);

// Frictional heating model
PtrList<incompressibleFrictionModel> frictionModelFluid(fluidRegions.size());

// Dynamic Mesh control variables
List<bool> correctPhiList(fluidRegions.size());
List<bool> checkMeshCourantNoList(fluidRegions.size());
List<bool> moveMeshOuterCorrectorsList(fluidRegions.size());

List<bool> frozenFlowFluid(fluidRegions.size(), false);

PtrList<IOMRFZoneList> MRFfluid(fluidRegions.size());
PtrList<fv::options> fluidFvOptions(fluidRegions.size());

// Populate fluid field pointer lists
forAll(fluidRegions, i)
{
    Info<< "*** Reading fluid mesh thermophysical properties for region "
        << fluidRegions[i].name() << nl << endl;

    Info<< "    Adding to velocity UFluid\n" << endl;
    UFluid.set
    (
        i,
        new volVectorField
        (
            IOobject
            (
                "U",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            ),
            fluidRegions[i]
        )
    );

    Info<< "    Adding to flux phiFluid\n" << endl;
    phiFluid.set
    (
        i,
        new surfaceScalarField
        (
            IOobject
            (
                "phi",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::NO_WRITE
            ),
            linearInterpolate(UFluid[i]) & fluidRegions[i].Sf()
        )
    );
    Info<< "    Adding to temperature field TFluid\n" << endl;
    TFluid.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "temp",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            ),
            fluidRegions[i]
        )
    );

	Info<< "    Adding to effective viscosity muEff\n" << endl;
    muEffl.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "muEff",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
			fluidRegions[i],
			dimensionedScalar("muEff", dimensionSet(1, -1, -1, 0, 0, 0, 0), 0.0)
        )
    );

	Info<< "    Adding to equivalent strain rate epsilonDotEq\n" << endl;
    epsilonDotEql.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "epsilonDotEq",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
			fluidRegions[i],
			dimensionedScalar("epsilonDotEq", dimless/dimTime, 0.0)
        )
    );

	// Create two phase transport model for each fluid region
	Info<< "	Creating two phase transport model\n" << endl;
	twoPhaseFluid.set
	(
		i,
		new immiscibleIncompressibleTwoPhaseThermalMixture(UFluid[i], phiFluid[i])
	);

	Info<< "	Adding to phase fraction alpha\n" << endl;

	// Populate volume fraction fields
	alpha1l.set
	(
		i,
		new volScalarField
		(
			"alpha1",
			twoPhaseFluid[i].alpha1()
		)
	);

	alpha2l.set
	(
		i,
		new volScalarField
		(
			"alpha2",
			twoPhaseFluid[i].alpha2()
		)
	);

	Info<< "    Calculating average fluid thermal conductivity kFluid\n" << endl;
    kFluid.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "k",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
			fluidRegions[i],
            dimensionedScalar("k", dimPower/dimLength/dimTemperature, 0.0)
        )
    );

	Info<< "    Adding to density rhoFluid\n" << endl;
    rhoFluid.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "rho",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
			fluidRegions[i],
            dimensionedScalar("rho", dimDensity, 0.0)
        )
    );

	Info<< "    Adding to specific heat cpFluid\n" << endl;
    cpFluid.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "cp",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::AUTO_WRITE
            ),
			fluidRegions[i],
            dimensionedScalar("cp", dimEnergy/dimMass/dimTemperature, 0.0)
        )
    );

	// Construct mass flux
	rhoPhiFluid.set
	(
		i,
		new surfaceScalarField
		(
			IOobject
			(
				"rhoPhi",
				runTime.timeName(),
				fluidRegions[i],
				IOobject::NO_READ,
				IOobject::NO_WRITE
			),
			fvc::interpolate(rhoFluid[i])*phiFluid[i]
		)
	);

	// Need to store rho*cp for ddt(rho*cp, T)
	// Initialization doesn't matter because rhoCp is reset after the gamma solution before use
	Info << "	Calculating rhoCp\n" << endl;
	rhoCpFluid.set
	(
		i,
		new volScalarField
		(
			IOobject
			(
				"rhoCp",
				runTime.timeName(),
				fluidRegions[i],
				IOobject::NO_READ,
				IOobject::NO_WRITE
			),
			fluidRegions[i],
			dimensionedScalar("rhoCp", dimEnergy/pow(dimLength,3)/dimTemperature, 0.0)
		)
	);

	// Energy flux
	// Again, initialization doesn't matter as it is reset after gamma calculation
	Info << "	Calculating rhoPhiCp\n" << endl;
	rhoPhiCpFluid.set
	(
		i,
		new surfaceScalarField
		(
			IOobject
			(
				"rhoPhiCp",
				runTime.timeName(),
				fluidRegions[i],
				IOobject::NO_READ,
				IOobject::NO_WRITE
			),
			rhoPhiFluid[i]*twoPhaseFluid[i].cp1()
		)
	);

	// Create incompressible turbulence model for each fluid region
    Info<< "    Adding to turbulenceFluid\n" << endl;
    turbulenceFluid.set
    (
        i,
		autoPtr<incompressible::turbulenceModel>
		(
		    incompressible::turbulenceModel::New
		    (
		        UFluid[i],
		        phiFluid[i],
		        twoPhaseFluid[i]
			)
		).ptr()
    );

	Info<< "    Adding to gFluid\n" << endl;
    gFluid.set
    (
        i,
        new uniformDimensionedVectorField
        (
            IOobject
            (
                "g",
                runTime.constant(),
                fluidRegions[i],
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            )
        )
    );

    Info<< "    Adding to hRefFluid\n" << endl;
    hRefFluid.set
    (
        i,
        new uniformDimensionedScalarField
        (
            IOobject
            (
                "hRef",
                runTime.constant(),
                fluidRegions[i],
                IOobject::READ_IF_PRESENT,
                IOobject::NO_WRITE
            ),
            dimensionedScalar("hRef", dimLength, Zero) // uses name
        )
    );

    dimensionedScalar ghRef
    (
        mag(gFluid[i].value()) > SMALL
      ? gFluid[i]
          & (cmptMag(gFluid[i].value())/mag(gFluid[i].value()))*hRefFluid[i]
      : dimensionedScalar("ghRef", gFluid[i].dimensions()*dimLength, 0)
    );

    Info<< "    Adding to ghFluid\n" << endl;
    ghFluid.set
    (
        i,
        new volScalarField
        (
            "gh",
            (gFluid[i] & fluidRegions[i].C()) - ghRef
        )
    );

    Info<< "    Adding to ghfFluid\n" << endl;
    ghfFluid.set
    (
        i,
        new surfaceScalarField
        (
            "ghf",
            (gFluid[i] & fluidRegions[i].Cf()) - ghRef
        )
    );

	Info<< "    Adding to p_rghFluid\n" << endl;
    p_rghFluid.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "p_rgh",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            ),
            fluidRegions[i]
        )
    );

	Info<< "    Adding to pFluid\n" << endl;
    pFluid.set
    (
        i,
        new volScalarField
        (
            IOobject
            (
                "p",
                runTime.timeName(),
                fluidRegions[i],
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            ),
            fluidRegions[i]
        )
    );

    // Force p_rgh to be consistent with p
	Info<< "	Enforcing consistency between p and p_rgh\n" << endl;
    p_rghFluid[i] = pFluid[i] - rhoFluid[i]*ghFluid[i];

	Info<< "	Checking if flux required\n" << endl;
    fluidRegions[i].setFluxRequired(p_rghFluid[i].name());

    Info<< "	Adding to radiationFluid\n" << endl;
	radiationFluid.set
    (
        i,
		autoPtr<radiation::radiationModel>
		(
		    radiation::radiationModel::New
		    (
		        TFluid[i]
			)
		).ptr()
    );

    const dictionary& pimpleDict = fluidRegions[i].solutionDict().subDict("PIMPLE");
    pimpleDict.readIfPresent("frozenFlow", frozenFlowFluid[i]);
	
	// Added to replace createDymControls.H
	pimpleDict.readIfPresent("correctPhi", correctPhiList[i]);
	pimpleDict.readIfPresent("checkMeshCourantNo", checkMeshCourantNoList[i]);
	pimpleDict.readIfPresent("moveMeshOuterCorrectors", moveMeshOuterCorrectorsList[i]);

    setRefCell
    (
        p_rghFluid[i],
        fluidRegions[i].solutionDict().subDict("PIMPLE"),
        pRefCellFluid[i],
        pRefValueFluid[i]
    );

	transportPropertiesl.set
	(
		i,
		new IOdictionary
		(
			IOobject
			(
				"transportProperties",
				runTime.constant(),
				fluidRegions[i],
				IOobject::MUST_READ_IF_MODIFIED,
				IOobject::NO_WRITE
			)
		)
	);

	UtFluid.set
	(
		i,
		new volVectorField
		(
			IOobject
			(
				"Ut",
				runTime.timeName(),
				fluidRegions[i],
				IOobject::NO_READ,
				IOobject::NO_WRITE
			),
			fluidRegions[i],
			Vt
		)
	);

    Info<< "    Creating stickModel for fluid\n" << endl;

	frictionModelFluid.set
	(
		i,
		new incompressibleFrictionModel(UFluid[i], UtFluid[i])
	);

	// Create stress and strain variables if desired
	#include "createStressStrain.H"

	// Create particle tracking phase if desired
	#include "createBeta.H"

    Info<< "    Adding MRF\n" << endl;
    MRFfluid.set
    (
        i,
        new IOMRFZoneList(fluidRegions[i])
    );

    Info<< "    Adding fvOptions\n" << endl;
    fluidFvOptions.set
    (
        i,
        new fv::options(fluidRegions[i])
    );

    turbulenceFluid[i].validate();

	// MULES compressed flux left out, don't think it is needed (see interfoam)

}
