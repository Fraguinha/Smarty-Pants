/* Libraries */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/* Definitions */

#define inputs (28 * 28) // the inputs available to make a prediction
#define outputs 10       // the outputs available as a prediction

#define total_weights (inputs * layer_size + layers * (layer_size * layer_size) + layer_size * outputs)
#define total_biases (layer_size + layers * layer_size + outputs)

/* Tweaking */
#define layers 1      // number of layers
#define layer_size 16 // number of neurons for each layer

#define min_weight -10 // minimum value for the weights
#define max_weight 10  // maximum value for the weights

#define min_bias -10 // maximum value for the bias
#define max_bias 10  // maximum value for the bias

#define training 60000
#define testing 10000

#define batch_size 100 // number of examples per batch
#define train_sessions (training / batch_size)

/* Structures */

/* Sensor */
typedef struct
{
    float activation;

} sensor;

/* Neuron */
typedef struct
{
    float bias;
    float weights[inputs];

    float error_derivative[batch_size];

    float bias_derivative;
    float weight_derivative[inputs];

    float weighted_sum;
    float activation;

} sensor_neuron;

/* Neuron */
typedef struct
{
    float bias;
    float weights[layer_size];

    float error_derivative[batch_size];

    float bias_derivative;
    float weight_derivative[layer_size];

    float weighted_sum;
    float activation;

} neuron_neuron;

/* Neural Network */
typedef struct
{
    /* Structure */
    sensor array_sensors[inputs];               // sensors
    sensor_neuron array_sn[layer_size];         // sensor neurons
    neuron_neuron array_nn[layers][layer_size]; // layers of neuron neurons
    neuron_neuron array_outputs[outputs];       // output neurons

    /* Cost */
    float cost_output[batch_size][outputs];

    /* Information */

    int prediction;
    int label;

} Neural_Network;

/* Function Declarations */

float randomizedFloat(float minimum, float maximum);

float sigmoid(float x);
float sigmoidDerivative(float x);

void save(Neural_Network *net, const char *filename);
void load(Neural_Network *net, const char *filename);

void randomize(Neural_Network *net);

void setInput(Neural_Network *net, const char *filename, const char *labelFilename, int x);
void showInputs(Neural_Network *net);
void showLabel(Neural_Network *net);

void feedForward(Neural_Network *net);

void showOutputs(Neural_Network *net);

void calculatePrediction(Neural_Network *net);
void showPrediction(Neural_Network *net);

void calculateCost(Neural_Network *net, int batch);

void backPropagate(Neural_Network *net);

void train(Neural_Network *net);

void getFilename(char *filename, int i, int f);

int main(int argc, char const *argv[])
{
    // Create the Neural Network
    Neural_Network smarty_pants;

    if (argc == 1)
    {
        // Randomize Neurons
        randomize(&smarty_pants);

        // Train network
        printf("Learning from %d images\n", training);
        train(&smarty_pants);

        // Save Network
        printf("Saving network to smarty_pants.bin\n");
        save(&smarty_pants, "smarty_pants.bin");
    }

    if (argc == 2)
    {
        // Load Network
        load(&smarty_pants, argv[1]);

        char filename[41];
        int correct = 0;

        // Test on all test images
        printf("Testing on %d images\n", testing);
        for (int i = 1; i <= testing; i++)
        {
            // Get filename for image
            getFilename(filename, i, 1);

            // Set Activation
            setInput(&smarty_pants, filename, "data/mnist-test-labels.txt", i);

            // Calculate activations
            feedForward(&smarty_pants);

            // Calculate prediction
            calculatePrediction(&smarty_pants);

            if (smarty_pants.label == smarty_pants.prediction)
            {
                correct++;
            }
            else
            {
                printf("Failed: %05d Predicted: %d Correct: %d\n", i, smarty_pants.prediction, smarty_pants.label);
            }
        }

        float precision = ((float)correct / testing) * 100;

        printf("Neural network effectiveness: %.2f%%\n", precision);
    }

    if (argc > 2)
    {
        // Load Network
        load(&smarty_pants, argv[1]);

        char filename[41];
        int test;

        if (argc == 3)
        {
            printf("Predicting 1 image\n");
        }
        else
        {
            printf("Predicting %d images\n", argc - 2);
        }

        // Test on argument images
        for (int i = 2; i < argc; i++)
        {
            // Get int
            sscanf(argv[i], "%d", &test);

            // Get filename for test image
            getFilename(filename, test, 1);

            // Set Activation
            setInput(&smarty_pants, filename, "data/mnist-test-labels.txt", test);

            // Calculate activations
            feedForward(&smarty_pants);

            // Show output
            showOutputs(&smarty_pants);

            // Calculate prediction
            calculatePrediction(&smarty_pants);

            // Show label
            showLabel(&smarty_pants);

            // Show prediction
            showPrediction(&smarty_pants);
        }
    }

    return 0;
}

/*
 * The sigmoid function
 * 
 * @param float x
 * @return float sigmoid
 */
float sigmoid(float x)
{
    float ex = expf(-x);

    float sigmoid = 1 / (1 + ex);

    return sigmoid;
}

/*
 * The sigmoid derivative function
 * 
 * @param float x
 * @return float sigmoid
 */
float sigmoidDerivative(float x)
{
    float ex = expf(-x);

    float derivative = ex / powf((1 + ex), 2);

    return derivative;
}

/*
 *  This function generates a random float
 *
 *  @param float minimum, float maximum
 *  @return float random
 */
float randomizedFloat(float minimum, float maximum)
{
    float random;
    random = (((float)rand()) / (float)(RAND_MAX)) * (maximum - minimum) + minimum;
    return random;
}

/*
 *  This functions saves the network to a binary file
 *
 *  @param Neural_Network *net, const char *filename
 */
void save(Neural_Network *net, const char *filename)
{
    FILE *fp = fopen(filename, "wb");

    if (fp != NULL)
    {
        fwrite(net, sizeof(Neural_Network), 1, fp);
    }

    fclose(fp);
}

/*
 *  This functions loads the network from a binary file
 *
 *  @param Neural_Network *net, const char *filename
 */
void load(Neural_Network *net, const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp != NULL)
    {
        fread(net, sizeof(Neural_Network), 1, fp);
    }

    fclose(fp);
}

/*
 *  This functions randomizes all the weights and biases of the neurons in the network
 *
 *  @param Neural_Network *net
 */
void randomize(Neural_Network *net)
{
    // Make randomizer
    srand((unsigned int)time(NULL));

    // Randomize sensor neurons weights and biases
    for (int i = 0; i < layer_size; i++)
    {
        net->array_sn[i].bias = randomizedFloat(min_bias, max_bias);

        for (int j = 0; j < inputs; j++)
        {
            net->array_sn[i].weights[j] = randomizedFloat(min_weight, max_weight);
        }
    }

    // Randomize hidden layer neurons weights and biases
    for (int i = 0; i < layers; i++)
    {
        for (int j = 0; j < layer_size; j++)
        {
            net->array_nn[i][j].bias = randomizedFloat(min_bias, max_bias);

            for (int k = 0; k < layer_size; k++)
            {
                net->array_nn[i][j].weights[k] = randomizedFloat(min_weight, max_weight);
            }
        }
    }

    // Randomize output neurons weights and biases
    for (int i = 0; i < outputs; i++)
    {
        net->array_outputs[i].bias = randomizedFloat(min_bias, max_bias);

        for (int j = 0; j < layer_size; j++)
        {
            net->array_outputs[i].weights[j] = randomizedFloat(min_weight, max_weight);
        }
    }
}

/*
 *  This functions sets the activations of all the sensors in the network, and the label
 *
 *  @param Neural_Network *net, const char *filename, const char *labelFilename, int i
 */
void setInput(Neural_Network *net, const char *filename, const char *labelFilename, int x)
{
    FILE *image = fopen(filename, "r");

    for (int i = 0; i < inputs; i++)
    {
        fscanf(image, "%f", &net->array_sensors[i].activation);
    }

    fclose(image);

    FILE *solution = fopen(labelFilename, "r");

    int trash;

    for (int j = 1; j < x; j++)
    {
        fscanf(solution, "%d", &trash);
    }

    fscanf(solution, "%d", &net->label);

    fclose(solution);
}

/*
 *  This functions shows the input activations
 *
 *  @param Neural_Network *net
 */
void showInputs(Neural_Network *net)
{
    for (int i = 0; i < inputs; i++)
    {
        printf("Activation of input [%03d]: %.2f\n", i, net->array_sensors[i].activation);
    }
}

/*
 *  This functions shows the correct label
 *
 *  @param Neural_Network *net
 */
void showLabel(Neural_Network *net)
{
    printf("Neural network correct label: %d\n", net->label);
}

/*
 *  This functions calculates the weighted sum of all the neurons in the network
 *  (weighted sum = sum(weights * activations) - bias)
 *
 *  @param Neural_Network *net
 */
void feedForward(Neural_Network *net)
{
    float sum;

    // Calculate sensor neurons weighted sum
    for (int i = 0; i < layer_size; i++)
    {
        sum = 0;

        for (int j = 0; j < inputs; j++)
        {
            sum += net->array_sensors[j].activation * net->array_sn[i].weights[j];
        }

        sum += net->array_sn[i].bias;

        net->array_sn[i].weighted_sum = sum;

        sum = sigmoid(sum);

        net->array_sn[i].activation = sum;
    }

    // Calculate hidden layer neurons weighted sum
    for (int i = 0; i < layers; i++)
    {
        for (int j = 0; j < layer_size; j++)
        {
            sum = 0;

            for (int k = 0; k < layer_size; k++)
            {
                sum += net->array_sn[k].weighted_sum * net->array_nn[i][j].weights[k];
            }

            sum += net->array_nn[i][j].bias;

            net->array_nn[i][j].weighted_sum = sum;

            sum = sigmoid(sum);

            net->array_nn[i][j].activation = sum;
        }
    }

    // Calculate output neurons weighted sum
    for (int i = 0; i < outputs; i++)
    {
        sum = 0;

        for (int j = 0; j < layer_size; j++)
        {
            sum += net->array_nn[layers - 1][j].weighted_sum * net->array_outputs[i].weights[j];
        }

        sum += net->array_outputs[i].bias;

        net->array_outputs[i].weighted_sum = sum;

        sum = sigmoid(sum);

        net->array_outputs[i].activation = sum;
    }
}

/*
 *  This function runs through the output neurons and calculates the one with the highest activation
 *
 *  @param Neural_Network *net
 */
void calculatePrediction(Neural_Network *net)
{
    float max = net->array_outputs[0].activation;
    int prediction = 0;

    for (int i = 0; i < outputs; i++)
    {
        if (net->array_outputs[i].activation > max)
        {
            max = net->array_outputs[i].activation;
            prediction = i;
        }
    }

    net->prediction = prediction;
}

/*
 *  This function shows the prediction
 *
 *  @param Neural_Network *net
 */
void showPrediction(Neural_Network *net)
{
    printf("Neural network prediction: %d\n", net->prediction);
}

/*
 *  This functions shows the outputs activations
 *
 *  @param Neural_Network *net
 */
void showOutputs(Neural_Network *net)
{
    for (int i = 0; i < outputs; i++)
    {
        printf("Activation of output [%d]: %.2f\n", i, net->array_outputs[i].activation);
    }
}

/*
 *  This functions calculates the score of the network
 *
 *  @param Neural_Network *net, int batch
 */
void calculateCost(Neural_Network *net, int batch)
{
    float sum = 0;

    for (int i = 0; i < outputs; i++)
    {
        if (i == net->label)
        {
            sum += powf(1 - net->array_outputs[i].activation, 2);
        }
        else
        {
            sum += powf(0 - net->array_outputs[i].activation, 2);
        }

        sum = sum / 2;

        net->cost_output[batch][i] = sum;
    }
}

/*
 *  This functions 
 *
 *  @param Neural_Network *net, int batch
 */
void backPropagate(Neural_Network *net)
{
    // for each example in batch
    for (int i = 0; i < batch_size; i++)
    {
        // for each output neuron
        for (int j = 0; j < outputs; j++)
        {
            net->array_outputs[j].error_derivative[i] = net->cost_output[i][j] * sigmoidDerivative(net->array_outputs[j].weighted_sum);
        }

        // for each layer neurons
        for (int l = layers; l >= 0; l--)
        {
            // for each neuron in layer
            for (int j = 0; j < layer_size; j++)
            {
                float sum = 0;

                // for each neuron in layer + 1
                for (int k = 0; k < layer_size; k++)
                {
                    if (l == layers && k <= outputs)
                    {
                        sum += net->array_outputs[k].weights[j] * net->array_outputs[k].error_derivative[i] * sigmoidDerivative(net->array_nn[l][j].weighted_sum);
                    }
                    else
                    {
                        if (l != layers)
                        {
                            sum += net->array_nn[l + 1][k].weights[j] * net->array_nn[l + 1][k].error_derivative[i] * sigmoidDerivative(net->array_nn[l][j].weighted_sum);
                        }
                    }
                }

                net->array_nn[l][j].error_derivative[i] = sum;
            }
        }

        // for each sensor neuron
        for (int j = 0; j < layer_size; j++)
        {
            float sum = 0;

            // for each neuron in layer + 1
            for (int k = 0; k < layer_size; k++)
            {
                sum += net->array_sn[k].weights[j] * net->array_sn[k].error_derivative[i] * sigmoidDerivative(net->array_sn[j].weighted_sum);
            }

            net->array_sn[j].error_derivative[i] = sum;
        }
    }
}

/*
 *  This functions makes the nextwork train over all examples
 *
 *  @param Neural_Network *net
 */
void train(Neural_Network *net)
{
    char filename[41];

    // make batches of training examples
    for (int i = 0; i < train_sessions; i++)
    {
        for (int j = 1; j <= batch_size; j++)
        {
            // Get filename for training image
            getFilename(filename, (i * batch_size) + j, 0);

            // Set Activation of input neurons
            setInput(net, filename, "data/mnist-train-labels.txt", i);

            // Calculate activations
            feedForward(net);

            // Calculate cost per example
            calculateCost(net, j - 1);
        }

        // Calculate error
        backPropagate(net);
    }
}

/*
 *  This functions creates filename string of image
 *
 *  @param Neural_Network *net, const char *filename, int i, int f 
 */
void getFilename(char *filename, int i, int f)
{

    filename[0] = '\0';

    char num[6];

    char *directory = "data/mnist-train-images/txt/";
    char *extension = ".tif.txt";

    if (f == 1)
    {
        char *directory = "data/mnist-test-images/txt/";
    }

    sprintf(num, "%05d", i);

    strcat(filename, directory);
    strcat(filename, num);
    strcat(filename, extension);
}