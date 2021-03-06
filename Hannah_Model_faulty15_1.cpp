// An Example Model for running the SPIKE simulator
//
// Authors: Nasir Ahmad (16/03/2016), James Isbister (23/3/2016), Gisbert Teepe [small position changes only](15/05/17) 

// To create the executable for this network, run:


#include <vector>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <getopt.h>
//#include <boost/filesystem.hpp>
#include<sys/stat.h>

using namespace std;


#include "Spike/Spike.hpp"
#include "UtilityStuff.hpp"


/*
 *	Main function in which the network is created and run
 */
int main (int argc, char *argv[]){

	
	//Options we can set to run with Dakota
	float layerwise_conductance_E2E_FF = 0.0025;
	float layerwise_conductance_E2I_L = 0.0875;
	float layerwise_conductance_I2E_L = 0.1;

	while (true)
	{
      int c;
      static struct option long_options[] =
        {
          /* These options set a flag. */
       	  //name, req argument. flag (ignore), unique value
          //{"tau_d", required_argument, 0, 0},
          //{"tau_c", required_argument, 0, 1},
          {"layerwiseE2EFF", required_argument, 0, 'a'},
          {"layerwiseE2IL", required_argument, 0, 'b'},
          {"layerwiseI2EL", required_argument, 0, 'd'},
          {0,0,0,0}

        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long(argc, argv, "", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        //the case is the unique value stored earlier. This is what actually changes the variable's value. 
        case 'a':
          //printf("tau_d entered: %s\n", optarg);
       		printf("E2E_FF entered: %s\n", optarg);
          //decay_term_tau_D = std::stof(optarg);
        	layerwise_conductance_E2E_FF = std::stof(optarg);
          //printf("tau_d set: %f\n", decay_term_tau_D);
        	printf("E2E_FF set: %f\n", layerwise_conductance_E2E_FF);
        	break;
        case 'b':
        	printf("E2I_L entered: %s\n", optarg);
        	layerwise_conductance_E2I_L = std::stof(optarg);
        	printf("E2I_L set: %f\n", layerwise_conductance_E2I_L);
        	break;
        case 'd':
        	printf("I2E_L entered: %s\n", optarg);
        	layerwise_conductance_I2E_L = std::stof(optarg);
        	printf("I2E_L set: %f\n", layerwise_conductance_I2E_L);
        	break;

        default:
        	abort ();
        }
    }

    /*
	 *
	 *	General Simulation Settings and Parameters
	 *
	 */

    int starting_epoch = 1;
	int lr_stop_epoch = 200;
	

	// Input of simulation name, number of epochs, and input folder name
	std::string modelpath = "../";
	std::string modelname = "Hannah_Model.cpp";
	//string name_of_current_stimulation = "20Ex20Id_dakota";//argv[1];
    string name_of_current_stimulation = "obj150face450_readable";

	// Files/Paths relevant to the input set
	std::string filepath = "../Data/MatlabGaborFilter/";
	std::string test_filelist = "../150face/FileList.txt"; 
	std::string train_filelist = "../face450/FileList_train.txt";


	// Allow the user to input the number of epochs for which the network should be trained.
	//cout << "Enter the times the network should be trained: \n";
	int input_epochs = 20;//std::stoi(argv[2]);
	//std::cin >> input_epochs;
	int number_of_epochs_train = input_epochs;

	// Finally the experiment name is set up to be a combination of the stimulation name and number of epochs
	string experimentName = name_of_current_stimulation + string("_") + string(to_string(input_epochs)) + string("_epochs");
	string inputs_for_test_name = "150face";
	string inputs_for_train_name = "face450";



	//bool simulate_network_to_test_untrained = true;
	bool simulate_network_to_train_network = true;
	bool simulate_network_to_test_trained = true;
	//bool human_readable_storage = true;

	// Since the model can be run under different connectivity styles, these booleans turn them on/off
	bool E2E_FB_ON = true;
	bool E2E_L_ON = true;
	bool E2E_L_STDP_ON = true;

	// In order to set up a sensible set of FF exc and inh values, a set of booleans have been set up to turn on/off the values
	bool inh_layer_on[] = {true, true, true, true};

	// Parameters for testing
	const float presentation_time_per_stimulus_per_epoch_test = 2.0f; // seconds
	bool record_spikes_test = true;
	bool save_recorded_spikes_and_states_to_file_test = true;
	// Parameters for training
	float presentation_time_per_stimulus_per_epoch_train = 0.2f; // 4.0f didn't really work; //used to be 2.0//0.2;//2.0f; // seconds

	/*
	 *
	 *	Visual Model General Settings. These are particular to whatever might be being changed in Dakota
	 *
	 */

	if (fanInCount_E2E_FF%max_number_of_connections_per_pair!=0){
		printf("total_number_of_new_synapses has to be a multiple of max_number_of_connections_per_pair");
		return 0;
	}

	float layerwise_biological_conductance_scaling_constant_lambda_E2E_FF[number_of_layers-1] = {
		layerwise_conductance_E2E_FF,  //0.625f gisi value
		layerwise_conductance_E2E_FF, //0.5f gisi value
		layerwise_conductance_E2E_FF};  //0.75f gisi value	
		/*
		original_timestep * layerwise_conductance_E2E_FF,  //0.625f gisi value
		original_timestep * layerwise_conductance_E2E_FF, //0.5f gisi value
		original_timestep * layerwise_conductance_E2E_FF};  //0.75f gisi value
		*/	

	float layerwise_biological_conductance_scaling_constant_lambda_E2I_L[number_of_layers] = {
		layerwise_conductance_E2I_L, //1.1f gisi value
		layerwise_conductance_E2I_L, //1.625f gisi value
		layerwise_conductance_E2I_L, //0.875f gisi value
		layerwise_conductance_E2I_L}; //1.6f gisi value
		/*
		original_timestep * layerwise_conductance_E2I_L, //1.1f gisi value
		original_timestep * layerwise_conductance_E2I_L, //1.625f gisi value
		original_timestep * layerwise_conductance_E2I_L, //0.875f gisi value
		original_timestep * layerwise_conductance_E2I_L}; //1.6f gisi value
		*/

	float layerwise_biological_conductance_scaling_constant_lambda_I2E_L[number_of_layers] = {
		layerwise_conductance_I2E_L, //0.04f gisi value
		layerwise_conductance_I2E_L, //0.375f gisi value
		layerwise_conductance_I2E_L, //0.2f gisi value
		layerwise_conductance_I2E_L}; //0.325f gisi value
		/*
		original_timestep * layerwise_conductance_I2E_L, //0.04f gisi value
		original_timestep * layerwise_conductance_I2E_L, //0.375f gisi value
		original_timestep * layerwise_conductance_I2E_L, //0.2f gisi value
		original_timestep * layerwise_conductance_I2E_L}; //0.325f gisi value
		*/	


	/*
	 *
	 *	Defining the Spiking Model
	 *
	 */

	// Create the SpikingModel
	SpikingModel* model = new SpikingModel();
	model->SetTimestep(timestep);

	//??? find out what new operator does 
	LIFSpikingNeurons* lif_spiking_neurons = new LIFSpikingNeurons();
	ImagePoissonInputSpikingNeurons* input_neurons = new ImagePoissonInputSpikingNeurons();
	ConductanceSpikingSynapses* conductance_spiking_synapses = new ConductanceSpikingSynapses();

	model->spiking_neurons = lif_spiking_neurons;
	model->input_spiking_neurons = input_neurons;
	model->spiking_synapses = conductance_spiking_synapses;

    // Creating an activity monitor for lif and input neurons
    //value of proportion_of_device_spike_store_full_before_copy within SpikingActivityMonitor.hpp can be changed below
    SpikingActivityMonitor* spike_monitor = new SpikingActivityMonitor(lif_spiking_neurons);
    spike_monitor->advanced_parameters->proportion_of_device_spike_store_full_before_copy = 0.20f;
    spike_monitor->advanced_parameters->device_spike_store_size_multiple_of_total_neurons = 100;
    spike_monitor->advanced_parameters->number_of_timesteps_per_device_spike_copy_check = 20;
    model->AddActivityMonitor(spike_monitor);
    SpikingActivityMonitor* input_spike_monitor = new SpikingActivityMonitor(input_neurons);
    input_spike_monitor->advanced_parameters->proportion_of_device_spike_store_full_before_copy = 0.20f;
    input_spike_monitor->advanced_parameters->device_spike_store_size_multiple_of_total_neurons = 100;
    input_spike_monitor->advanced_parameters->number_of_timesteps_per_device_spike_copy_check = 20;
    model->AddActivityMonitor(input_spike_monitor);

    // STDP Rule Parameters
    // ??? look into precise meanings of stdp parameters
	evans_stdp_plasticity_parameters_struct STDP_PARAMS;
	STDP_PARAMS.decay_term_tau_C = decay_term_tau_C;
	STDP_PARAMS.decay_term_tau_D = decay_term_tau_D;
	STDP_PARAMS.model_parameter_alpha_D = 0.5;
	STDP_PARAMS.synaptic_neurotransmitter_concentration_alpha_C = 0.5*2.0f;
	STDP_PARAMS.learning_rate_rho = learning_rate_rho;
	EvansSTDPPlasticity* evans_stdp = new EvansSTDPPlasticity(conductance_spiking_synapses, lif_spiking_neurons, input_neurons, &STDP_PARAMS);

	model->AddPlasticityRule(evans_stdp);

	// Creating the input neurons
	TimerWithMessages adding_input_neurons_timer("Adding Input Neurons...\n");

	// Loading the required files
	// set_up_rates and AddGroup... are part of ImagePoissonSpikingNeurons.cpp, as these are my input neurons ***these were test_name
	input_neurons->set_up_rates(train_filelist.c_str(), "FilterParameters.txt", (filepath+inputs_for_train_name+"/").c_str(), max_FR_of_input_Gabor);

	image_poisson_input_spiking_neuron_parameters_struct image_poisson_input_spiking_group_params;
	image_poisson_input_spiking_group_params.rate = 30.0f;
	input_neurons->AddGroupForEachGaborType(&image_poisson_input_spiking_group_params);

	adding_input_neurons_timer.stop_timer_and_log_time_and_message("Input Neurons Added.", true);

	// Neuron Layer Creation
	TimerWithMessages adding_neurons_timer("Adding Neurons...\n");

	lif_spiking_neuron_parameters_struct EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS;
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.group_shape[0] = dim_excit_layer;
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.group_shape[1] = dim_excit_layer;
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.resting_potential_v0 = -0.074f;
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.threshold_for_action_potential_spike = -0.053f;
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.somatic_capacitance_Cm = 500.0*pow(10, -12);
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.somatic_leakage_conductance_g0 = 25.0*pow(10, -9);
	EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS.absolute_refractory_period = absolute_refractory_period;


	lif_spiking_neuron_parameters_struct INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS;
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.group_shape[0] = dim_inhib_layer;
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.group_shape[1] = dim_inhib_layer;
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.resting_potential_v0 = -0.082f;
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.threshold_for_action_potential_spike = -0.053f;
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.somatic_capacitance_Cm = 214.0*pow(10, -12);
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.somatic_leakage_conductance_g0 = 18.0*pow(10, -9);
	INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS.absolute_refractory_period = absolute_refractory_period;

	vector<int> EXCITATORY_NEURONS;
	vector<int> INHIBITORY_NEURONS;
	for (int l=0;l<number_of_layers;l++){
		EXCITATORY_NEURONS.push_back(model->AddNeuronGroup(&EXCITATORY_LIF_SPIKING_NEURON_GROUP_PARAMS));
		INHIBITORY_NEURONS.push_back(model->AddNeuronGroup(&INHIBITORY_LIF_SPIKING_NEURON_GROUP_PARAMS));
		cout<<"Neuron Group "<<EXCITATORY_NEURONS[l]<<": Excitatory layer "<<l<<endl;
		cout<<"Neuron Group "<<INHIBITORY_NEURONS[l]<<": Inhibitory layer "<<l<<endl;
	}


	adding_neurons_timer.stop_timer_and_log_time_and_message("Neurons Added.", true);

	// Synapse Creation
	TimerWithMessages adding_synapses_timer("Adding Synapses...\n");


	conductance_spiking_synapse_parameters_struct G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[0] = timestep;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[1] = timestep;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.max_number_of_connections_per_pair = 1;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_per_postsynaptic_neuron = fanInCount_G2E_FF;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = biological_conductance_scaling_constant_lambda_G2E_FF;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;
	// In aki's model, learning on this set of synapses was off. Remove the line below to math that.
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.plasticity_vec.push_back(evans_stdp);
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_standard_deviation = gaussian_synapses_standard_deviation_G2E_FF;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.reversal_potential_Vhat = 0.0; //Volts
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.decay_term_tau_g = decay_term_tau_g_G2E_FF;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[0] = weight_range_bottom;
	G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[1] = weight_range_top;


	conductance_spiking_synapse_parameters_struct E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[0] = E2E_FF_minDelay;//5.0*timestep;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[1] = E2E_FF_maxDelay;//10.0f*pow(10, -3);
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.max_number_of_connections_per_pair = 1;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_per_postsynaptic_neuron = fanInCount_E2E_FF;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = biological_conductance_scaling_constant_lambda_E2E_FF;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.plasticity_vec.push_back(evans_stdp);
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.reversal_potential_Vhat = 0.0;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.decay_term_tau_g = decay_term_tau_g_E2E_FF;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[0] = weight_range_bottom;
	E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[1] = weight_range_top;


	conductance_spiking_synapse_parameters_struct E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS;
	if(E2E_FB_ON){
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[0] = E2E_FB_minDelay;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[1] = E2E_FB_maxDelay;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.max_number_of_connections_per_pair = 1;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_per_postsynaptic_neuron = fanInCount_E2E_FB;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = biological_conductance_scaling_constant_lambda_E2E_FB;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.plasticity_vec.push_back(evans_stdp);
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_standard_deviation = gaussian_synapses_standard_deviation_E2E_FB;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.reversal_potential_Vhat = 0.0;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.decay_term_tau_g = decay_term_tau_g_E2E_FB;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[0] = weight_range_bottom;
		E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[1] = weight_range_top;
	}


	conductance_spiking_synapse_parameters_struct E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[0] = E2I_L_minDelay; //5.0*timestep;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[1] = E2I_L_maxDelay; //10.0f*pow(10, -3);
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.max_number_of_connections_per_pair = 1;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_per_postsynaptic_neuron = fanInCount_E2I_L;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = biological_conductance_scaling_constant_lambda_E2I_L;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_standard_deviation = gaussian_synapses_standard_deviation_E2I_L;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.reversal_potential_Vhat = 0.0;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.decay_term_tau_g = decay_term_tau_g_E2I_L;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[0] = weight_range_bottom;
	E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[1] = weight_range_top;


	conductance_spiking_synapse_parameters_struct I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[0] = I2E_L_minDelay;//5.0*timestep;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[1] = I2E_L_maxDelay;//3.0f*pow(10, -3);
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.max_number_of_connections_per_pair = 1;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_per_postsynaptic_neuron = fanInCount_I2E_L;
	
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_standard_deviation = gaussian_synapses_standard_deviation_I2E_L;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.reversal_potential_Vhat = -70.0*pow(10, -3);
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.decay_term_tau_g = decay_term_tau_g_I2E_L;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[0] = weight_range_bottom;
	I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[1] = weight_range_top;

	conductance_spiking_synapse_parameters_struct E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS;
	if(E2E_L_ON){
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[0] = E2E_L_minDelay;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.delay_range[1] = E2E_L_maxDelay;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.max_number_of_connections_per_pair = 1;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_per_postsynaptic_neuron = fanInCount_E2E_L;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = biological_conductance_scaling_constant_lambda_E2E_L;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.connectivity_type = CONNECTIVITY_TYPE_GAUSSIAN_SAMPLE;
		if (E2E_L_STDP_ON)
		  E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.plasticity_vec.push_back(evans_stdp);
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_standard_deviation = gaussian_synapses_standard_deviation_E2E_L;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.reversal_potential_Vhat = 0.0;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.decay_term_tau_g = decay_term_tau_g_E2E_L;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[0] = weight_range_bottom;
		E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_range[1] = weight_range_top;
	}


	for (int l=0; l<number_of_layers; l++){

		if(l==0){
		  model->AddSynapseGroupsForNeuronGroupAndEachInputGroup(
				  EXCITATORY_NEURONS[l], 
				  &G2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS);
    	}
		else{

			E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.gaussian_synapses_standard_deviation = gaussian_synapses_standard_deviation_E2E_FF[l-1];
			E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = layerwise_biological_conductance_scaling_constant_lambda_E2E_FF[l-1];
			for (int connection_number = 0; connection_number < max_number_of_connections_per_pair; connection_number++){
				model->AddSynapseGroup(EXCITATORY_NEURONS[l-1], EXCITATORY_NEURONS[l], &E2E_FF_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS);
			}
			if(E2E_FB_ON){

				model->AddSynapseGroup(EXCITATORY_NEURONS[l], EXCITATORY_NEURONS[l-1], &E2E_FB_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS);
			}
		}

		E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = layerwise_biological_conductance_scaling_constant_lambda_E2I_L[l];
		model->AddSynapseGroup(EXCITATORY_NEURONS[l], INHIBITORY_NEURONS[l], &E2I_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS);
		if (inh_layer_on[l]){

			I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS.weight_scaling_constant = layerwise_biological_conductance_scaling_constant_lambda_I2E_L[l];
			model->AddSynapseGroup(INHIBITORY_NEURONS[l], EXCITATORY_NEURONS[l], &I2E_L_INHIBITORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS);
		}
		if(E2E_L_ON){

			model->AddSynapseGroup(EXCITATORY_NEURONS[l], EXCITATORY_NEURONS[l], &E2E_L_EXCITATORY_CONDUCTANCE_SPIKING_SYNAPSE_PARAMETERS);
		}
	}
	
	adding_synapses_timer.stop_timer_and_log_time_and_message("Synapses Added.", true);

	/*
	 *
	 *	Finalize and Run the model
	 *
	 */

	model->finalise_model();

	// Creating Relevant folders
	std::string exppath = "/Users/hakunahahannah/Documents/Projects/Spiking3.0/Build/output/" + experimentName;
	std::string initpath = "/Users/hakunahahannah/Documents/Projects/Spiking3.0/Build/output/" + experimentName + "/initial";
	std::string trainpath = "/Users/hakunahahannah/Documents/Projects/Spiking3.0/Build/output/" + experimentName + "/training";
	std::string testpath = "/Users/hakunahahannah/Documents/Projects/Spiking3.0/Build/output/" + experimentName + "/testing";

	mkdir(exppath.c_str(), ACCESSPERMS);
	mkdir(initpath.c_str(), ACCESSPERMS);
	mkdir(trainpath.c_str(), ACCESSPERMS);
	mkdir(testpath.c_str(), ACCESSPERMS);

	float equalize_target = 3.5f; // 0.19f;
	// Load the desired input stimuli and equalize their rate ??? why do we equalise the rate? ***these were test
	input_neurons->set_up_rates(train_filelist.c_str(), "FilterParameters.txt", (filepath+inputs_for_train_name+"/").c_str(), max_FR_of_input_Gabor);
	equalize_rates(input_neurons, equalize_target);
	input_neurons->copy_rates_to_device();

	// Run the untrained initial network

	printf("\n\n\n\n ------ INITIAL\n");
  
  	for (int stimulus_index = 0; stimulus_index < input_neurons->total_number_of_input_stimuli; stimulus_index++){
  		// for (int stimulus_index = 0; stimulus_index < 2; stimulus_index++){ (to test quickly)
  		input_neurons->select_stimulus(stimulus_index);
    	model->run(presentation_time_per_stimulus_per_epoch_test, false); // false for plasticity_on
    	model->spiking_neurons->reset_state(); //membrane voltages 
    	model->spiking_synapses->reset_state(); //and synaptic inputs are set to rest (to -60 and 0 respectively)

    	// Setting output name
  		string output_prefix = string("Initial_Output_") + string("_Stimulus_") + string(to_string(stimulus_index)) + string("_");
 		//spike_monitor->save_spikes_as_binary(initpath + "/", output_prefix.c_str());
 		spike_monitor->save_spikes_as_txt(initpath + "/", output_prefix.c_str());
 		spike_monitor->reset_state(); //  this means it's saving per stimulus 
 		string input_prefix = string("Initial_Input_") + string("_Stimulus_") + string(to_string(stimulus_index)) + string("_");
 		//input_spike_monitor->save_spikes_as_binary(initpath + "/", input_prefix.c_str());
 		input_spike_monitor->save_spikes_as_txt(initpath + "/", input_prefix.c_str());
 		input_spike_monitor->reset_state();
  	}

	// Saving Spikes to File. These are for the entirety of Initial and are not stimulus specific
    //spike_monitor->save_spikes_as_binary(initpath + "/", "Initial_");
	//input_spike_monitor->save_spikes_as_binary(initpath + "/", "Initial_Input_");
	spike_monitor->save_spikes_as_txt(initpath + "/", "Initial_");
	input_spike_monitor->save_spikes_as_txt(initpath + "/", "Initial_Input_");
	
	// removes the last thing entered into montiors; i.e. input_spike_monitor as this is no longer needed
	model->monitors_vec.pop_back();

	// Saving Synapse Structure to File
	//model->spiking_synapses->save_connectivity_as_binary(initpath + "/", "Initial_");
	model->spiking_synapses->save_connectivity_as_txt(initpath + "/", "Initial_");

	/*
	 *	Run network training and testing. Each training epoch is followed by a testing epoch for faster analysis
	 */

	// Loop through the number of epochs
	for (int g = starting_epoch; g <= number_of_epochs_train; g++){
		// Reset everything before another epoch
		model->reset_state();
		spike_monitor->reset_state();
		// input_spike_monitor->reset_state();
		// Carry out training (as necessary)
		printf("\n\n\n\n ------ TRAIN EPOCH; %d\n", g);
		if (simulate_network_to_train_network) {
			// Load the desired input stimuli and equalize their rate ***these were test
			input_neurons->set_up_rates(train_filelist.c_str(), "FilterParameters.txt", (filepath+inputs_for_train_name+"/").c_str(), max_FR_of_input_Gabor);
			equalize_rates(input_neurons, equalize_target);
			input_neurons->copy_rates_to_device();

			//learning rate increases by 1.1 each epoch but stops at a certain point, otherwise LR gets too high. As the initial output is random, a high LR means it learns incorrectly
			if (g != 1) 
				STDP_PARAMS.learning_rate_rho = powf(1.1f, (g-1))*learning_rate_rho;
			if (g >= lr_stop_epoch)
				STDP_PARAMS.learning_rate_rho = powf(1.1f, (lr_stop_epoch))*learning_rate_rho;

			// randomise stimulus order
		    std::vector<int> stimulus_order;
		    for (int stimulus_index = 0; stimulus_index < input_neurons->total_number_of_input_stimuli; stimulus_index++){
		  		stimulus_order.push_back(stimulus_index);
		  	}
		  	std::srand(g);
		  	std::random_shuffle ( stimulus_order.begin(), stimulus_order.end() );

		  	for (int stimulus_index = 0; stimulus_index < input_neurons->total_number_of_input_stimuli; stimulus_index++){
		 	//for testing use this line:::   for (int stimulus_index = 0; stimulus_index < 2; stimulus_index++){
		    	input_neurons->select_stimulus(stimulus_order[stimulus_index]);
		    	model->run(presentation_time_per_stimulus_per_epoch_train, true);
		    	spike_monitor->reset_state();
		    //the lines below need to commented out for trace and uncommented for CT
		    // model->spiking_neurons->reset_state();
		    // model->spiking_synapses->reset_state();
		  	}
			// Setting output name
		  string train_prefix = string("Train_Epoch") + string(to_string(g)) + string("_");
		  //model->spiking_synapses->save_weights_as_binary(trainpath + "/", train_prefix.c_str());
		  model->spiking_synapses->save_weights_as_txt(trainpath + "/", train_prefix.c_str());
		}

		// Reset everything before another epoch
		model->reset_state();
		spike_monitor->reset_state();
		// input_spike_monitor->reset_state();

		if (simulate_network_to_test_trained) {

			printf("\n\n\n\n ------ TEST EPOCH; %d\n", g);

			// Load the desired input stimuli and equalize their rate
			input_neurons->set_up_rates(test_filelist.c_str(), "FilterParameters.txt", (filepath+inputs_for_test_name+"/").c_str(), max_FR_of_input_Gabor);
			equalize_rates(input_neurons, equalize_target);
			input_neurons->copy_rates_to_device();

			for (int stimulus_index = 0; stimulus_index < input_neurons->total_number_of_input_stimuli; stimulus_index++){
		    	input_neurons->select_stimulus(stimulus_index);
		   		model->run(presentation_time_per_stimulus_per_epoch_test, false);
		   		model->spiking_neurons->reset_state(); // 
		    	model->spiking_synapses->reset_state();

		    	//create epoch folder
				std::string epochpath = "/Users/hakunahahannah/Documents/Projects/Spiking3.0/Build/output/" + experimentName + "/testing/epoch" + to_string(g);
				mkdir(epochpath.c_str(), ACCESSPERMS);

			    // Setting output name
			  	string test_prefix = string("Test_Epoch") + string(to_string(g)) + string("_Stimulus_") + string(to_string(stimulus_index)) + string("_");
			 	//spike_monitor->save_spikes_as_binary(epochpath + "/", test_prefix.c_str());
			 	spike_monitor->save_spikes_as_txt(epochpath + "/", test_prefix.c_str());
			 	spike_monitor->reset_state();

		    }
		}
	}

	return 0;
}





