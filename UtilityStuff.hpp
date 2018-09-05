
// The timestep at which this network is run is entered here.
// Note that the timestep can be set higher than usual for any period in which you want to generally test the network behaviour.
float timestep = 0.0002;
float original_timestep = 0.0000002;// 0.00002;//0.00002; //0.0000002; //it's actually 0.00002, but is multiplied by 0.001 as a dakota bug check
const int number_of_layers = 4;


// Network Parameters
// NEXT LINE REMOVED FOR THIS VERSION OF DAKOTA!!!!
//const int number_of_layers = 4;			// This value is explicitly assumed in this model. Not recommended to change unless you understand what else may need changing in this file.
int max_number_of_connections_per_pair = 2; // The maximum number of connections refers to multiple synaptic contacts pre->post
int dim_excit_layer = 64 / 2;			// The dimension of the excitatory layers (grid with this width)
int dim_inhib_layer = 32 / 2;			// The dimension of the inhibitory layers (as above)

// G2E = Gabor to excitatory, E2E = excitatory to excitatory, E2I = excitatory to inhibitory, I2E = inhibitory to excitatory
// FF = feed forward, L = Lateral, FB = Feedback

// Measure of the radius of the Fan-in	
float gaussian_synapses_standard_deviation_G2E_FF =  10.0; //gisi value 4.0
float gaussian_synapses_standard_deviation_E2E_FF[number_of_layers-1] = {50.0, 50.0, 50.0}; // List for each layer, can be customized
float gaussian_synapses_standard_deviation_E2E_FB = 8.0;
float gaussian_synapses_standard_deviation_E2E_L = 14.0;
float gaussian_synapses_standard_deviation_E2I_L = 8.0; //gisi value 4.0
float gaussian_synapses_standard_deviation_I2E_L = 4.0; //gisi value 8.0
		
// Fan-in Number 
int fanInCount_G2E_FF = 90; //90 gisi value
int fanInCount_E2E_FF = 60; //90 gisi value
int fanInCount_E2E_FB = 5; //5 gisi value
int fanInCount_E2E_L = 30; //30 gisi value
int fanInCount_E2I_L = 60; //60 gisi value
int fanInCount_I2E_L = 60; //90 gisi value
// Synaptic Parameters 
// Range of axonal transmission delay
// timestep is defined above 
float min_delay = 10.0*timestep; // In timesteps //was 5.0 but 10.0 speeds it up
float max_delay = 0.01; // In seconds (10ms)
float max_FR_of_input_Gabor = 100.0f; // Hz
float absolute_refractory_period = 0.002; // s
//Synaptic Parameters
float weight_range_bottom = 0.0;
float weight_range_top = 1.0;
float learning_rate_rho = 0.01f;

// calculating different Connections 
float E2E_FF_minDelay = min_delay; 
float E2E_FF_maxDelay = max_delay;//3.0f*pow(10, -3);
float E2I_L_minDelay = min_delay;
float E2I_L_maxDelay = max_delay;//3.0f*pow(10, -3);
float I2E_L_minDelay = min_delay;
float I2E_L_maxDelay = max_delay;//3.0f*pow(10, -3);
float E2E_FB_minDelay = min_delay;
float E2E_FB_maxDelay = max_delay;
float E2E_L_minDelay = min_delay;
float E2E_L_maxDelay = max_delay;

// Below are the decay rates of the variables for learning: Pre/Post synaptic activities C and D (See Ben Evans)
float decay_term_tau_C = 0.05; //aki_paper = 0.005 // 0.3(In Ben's model, tau_C/tau_D = 0.003/0.005 v 0.015/0.025 v 0.075/0.125, and the first one produces the best result)
float decay_term_tau_D = 0.05; //aki_paper = 0.005


// Biological Scaling Constant = How much you multiply the weights up or down for realism/stability
// If this value is roughly on the order of the Leakage Conductance, it will be close to one input spike -> one output spike (n.b. depends on syn tau)
float biological_conductance_scaling_constant_lambda_G2E_FF = 0.1 * 0.0001 * original_timestep;
float biological_conductance_scaling_constant_lambda_E2E_FF = 0.00005 * original_timestep;
float biological_conductance_scaling_constant_lambda_E2E_FB = 0.1 * 0.0001 * original_timestep;
float biological_conductance_scaling_constant_lambda_E2E_L	= 0.000001 * original_timestep;
float biological_conductance_scaling_constant_lambda_E2I_L	= 0.001 * original_timestep;
float biological_conductance_scaling_constant_lambda_I2E_L	= 0.005 * original_timestep;


// Tau G = Synaptic Conductance Decay TIME CONSTANT for each synapse type (#1) (Seconds)
// Most of these values are set to 150ms for trace-like learning. Other than Exc->Inh and Inh->Exc
float decay_term_tau_g_G2E_FF	=	0.15;
float decay_term_tau_g_E2E_FF	=	0.15;
float decay_term_tau_g_E2E_FB	=	0.15;
float decay_term_tau_g_E2E_L	=	0.15;
float decay_term_tau_g_E2I_L	=	0.002;
float decay_term_tau_g_I2E_L	=	0.025; //0.005;//In Ben's model, 0.005 v 0.025 and latter produced better result




/** Function to load weights from a file into a SpikingModel. 
*/
void load_weights(
		  SpikingModel* Model,		/**< SpikingModel Pointer to the model which should load weights */
		  std::string weightloc,	/**< String path to the file from which weights should be loaded */
		  bool binaryfile)		/**< Boolean flag indicating if the file is a binary file */
{
	std::ifstream weightfile;
	std::vector<float> WeightsToLoad; // This vector should ultimately hold the list of replacement weights

	if (binaryfile){
		weightfile.open (weightloc, ios::in | ios::binary);
		while( weightfile.good() )
		{
			float currentweight;
			weightfile.read((char*)&currentweight, sizeof(float));
			if (weightfile.eof()) {
				weightfile.close();
				break;
			}
			WeightsToLoad.push_back(currentweight);
		}
	} else {
		weightfile.open(weightloc);
		while( weightfile.good() )
		{
			string fileline;
			getline( weightfile, fileline);
			if (weightfile.eof()) {
				weightfile.close();
				break;
			}
			// Put each line into the float vector
			WeightsToLoad.push_back( std::stof(fileline) );
		}
	}
	// Check if you have the correct number of weights
	if (WeightsToLoad.size() != Model->spiking_synapses->total_number_of_synapses){
		printf("%d, %d\n", (int)WeightsToLoad.size(), Model->spiking_synapses->total_number_of_synapses);
		printf("The number of weights being loaded is not equivalent to the model.");
		exit(2);
	}
	// If the previous test is passed, apply the weights
	for (int i=0; i < WeightsToLoad.size(); i++){
		Model->spiking_synapses->synaptic_efficacies_or_weights[i] = WeightsToLoad[i];
	}
	printf("%ld Weights Loaded.\n", WeightsToLoad.size());
}


/** Function to equalize the mean rate of the stimuli being presented to the network.
 *	Not strictly necessary if the stimuli are set up well.
*/
void equalize_rates(
			ImagePoissonInputSpikingNeurons* input_neurons, /**< ImagePoissonInputSpikingNeuron pointer to initialized input population */
			float target)					/**< float value indicating desired mean FR */
{
	// Rates can be altered here without much issue
	int num_rates_per_image = input_neurons->total_number_of_rates_per_image;
	int num_images = input_neurons->total_number_of_input_stimuli;

	for (int image_index = 0; image_index < num_images; image_index++){
		float meanval = 0.0f;
		for (int rate_index = 0; rate_index < num_rates_per_image; rate_index++){
			meanval += input_neurons->gabor_input_rates[image_index*num_rates_per_image + rate_index];
		}
		meanval /= float(num_rates_per_image);
		// printf("%f\n", meanval);

		float multiplication_factor = target / meanval;
		for (int rate_index = 0; rate_index < num_rates_per_image; rate_index++){
			input_neurons->gabor_input_rates[image_index*num_rates_per_image + rate_index] *= multiplication_factor;
		}
	}
}

