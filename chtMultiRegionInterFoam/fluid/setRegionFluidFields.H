    // Dynamic mesh
	dynamicFvMesh& mesh = fluidRegions[i];

	// Include this to define LTS variable after mesh defined
	#include "createRDeltaT.H"

	// Mixture model
	immiscibleIncompressibleTwoPhaseThermalMixture& thermo = twoPhaseFluid[i];

	// Friction / stick model
	incompressibleFrictionModel& friction = frictionModelFluid[i];

	// Turbulence model
    incompressible::turbulenceModel& turbulence = turbulenceFluid[i];

	// Radiation model
    radiation::radiationModel& rad = radiationFluid[i];

	// Create pointers to basic fluid property fields
	volScalarField& p = pFluid[i];
	volScalarField& p_rgh = p_rghFluid[i];
	volScalarField& temp = TFluid[i];
    volVectorField& U = UFluid[i];
    surfaceScalarField& phi = phiFluid[i];

	// Pointers to gravity-related fields
    const volScalarField& gh = ghFluid[i];
    const surfaceScalarField& ghf = ghfFluid[i];

	// Pointers to viscosity and effective strain rate
	volScalarField& muEff = muEffl[i];
	volScalarField& epsilonDotEq = epsilonDotEql[i];

	// Phase fraction
    volScalarField& alpha1(thermo.alpha1());
	volScalarField& alpha2(thermo.alpha2());

	// Mixture thermal properties
	volScalarField& rho = rhoFluid[i];
	volScalarField& kf = kFluid[i];
	volScalarField& cpf = cpFluid[i];

	// Retrieve individual phase properties from mixture class
	dimensionedScalar rho1 = thermo.rho1();
	dimensionedScalar rho2 = thermo.rho2();
	dimensionedScalar cp1	= thermo.cp1();
	dimensionedScalar cp2  = thermo.cp2();

	// Flux terms for energy calculation
	surfaceScalarField& rhoPhi = rhoPhiFluid[i];
	surfaceScalarField& rhoPhiCp = rhoPhiCpFluid[i];
	volScalarField& rhoCp = rhoCpFluid[i];

	// Stress and strain fields and variables
	bool trackStressStrain = trackStressStrainl[i];
	volScalarField& Z = ZFluid[i];
	volScalarField& sigmaf = sigmafFluid[i];
	volScalarField& epsilonEq = epsilonEqFluid[i];
	dimensionedScalar& gammaEpsilon = gammaEpsilonFluid[i];
	volScalarField& sigmavm = sigmavmFluid[i];

	// Particle tracking / 2nd phase
	bool particleTracking = particleTrackingl[i];
	volScalarField& beta = betaFluid[i];
	dimensionedScalar& gammaBeta = gammaBetaFluid[i];

	// Misc. items
    IOMRFZoneList& MRF = MRFfluid[i];
    fv::options& fvOptions = fluidFvOptions[i];

    bool frozenFlow = frozenFlowFluid[i];

    const label pRefCell = pRefCellFluid[i];
    const scalar pRefValue = pRefValueFluid[i];

	// Alpha solver variables
	bool& alphaRestart = alphaRestartList[i];
	surfaceScalarField& alphaPhi10 = alphaPhi10List[i];
	tmp<surfaceScalarField> talphaPhi1Corr0;

	#include "createUfIfPresent.H"

	// Dynamic Mesh controls
	bool& correctPhi = correctPhiList[i];
	bool& checkMeshCourantNo = checkMeshCourantNoList[i];
	bool& moveMeshOuterCorrectors = moveMeshOuterCorrectorsList[i];

	pimpleControl pimple(mesh);

	#include "initCorrectPhi.H"
