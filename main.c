#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// amount to change on each step
#define LEARNING_CONST 0.05

	
#define TRAINING "train-images-idx3-ubyte"
#define LABELS "train-labels-idx1-ubyte"

typedef struct Image Image;
typedef struct Layer Layer;
typedef struct Perceptron Perceptron;
typedef struct Answer Answer;
typedef unsigned char Label;

// represents the image of one number
struct Image {
	unsigned char pixels[28*28];
};

// each perceptron has a weight for each pixel in an image
struct Perceptron {
	double weights[28*28];
	unsigned char *inputs;
	double output;
};

struct Layer {
	Perceptron cells[10];
};

struct Answer {
	int vect[10];
};

Image weightsToImage(Perceptron p){
	Image img;
	int i=0;
	for (i; i < 28*28; i++){
		img.pixels[i] = (int) (p.weights[i] * 255);
	}
	return img;
}

// prints an unsigned character based on a table of characters
// of varying darkness
void printValue(unsigned int n, int simpleFlag){
	if (simpleFlag){ printf("%c ", (n>0) ? 'X' : '.'); return; }

	char c[8] = {'.','-',':','!','$','#','@','@'};
	printf("%c ",c[n/(255/7)]); 

}

// method to print 28 x 28 grid 
void printImage(Image *image,int simpleFlag){
	int i=0;
	for (i; i<28; i++){
		int j=0;
		for (j; j<28;j++){
			printValue(image->pixels[(28*i) + j],simpleFlag);
		}
		printf("\n");
	}
}

// loads next image into *img
int getNextImage(Image *img,FILE *fd){
	if (fread(img,1,28*28,fd) != 28*28){
		printf("Done reading images.\n");
		return 0;
	}
	/*  This turns every pixel to 1 if it is not 0
	int i=0;
	for (i;i<28*28;i++){
		if (img->pixels[i]) img->pixels[i] = 1;
	}
	*/
	return 1;
}

// loads next label into *label
int getNextLabel(Label *label,FILE *fd){
	if (fread(label,1,1,fd) != 1){
		printf("Done reading labels.\n");
		return 0;
	}
	return 1;
}

Perceptron * newPerceptron(unsigned char * input){
	Perceptron *p = malloc(sizeof(Perceptron));
	p->inputs = input;
	int i=0;
	// randomize all weights on new perceptron
	for (i; i<28*28; i++){
		// random percentage
		p->weights[i] = (double) rand() / (double)(unsigned) RAND_MAX;
	}

	return p;

}

void updateOutput(Perceptron *p){
	p->output = 0;
	int i=0;
	for (i;i<28*28;i++){
		p->output += p->weights[i] * (double) p->inputs[i];
	}
	p->output /= (double)(28*28); // between 0 and 1
}

void updateWeight(Perceptron *p, int error){
	int i = 0;
	for (i; i<28*28;i++){
		// always increase or decrease by learning constant
		p->weights[i] += LEARNING_CONST *  p->inputs[i] * error;
		//p->weights[i] += LEARNING_CONST * ( (double)(p->inputs[i]) / (double)255 ) * error;
	}
}

void testLayer(Layer l){
	char *images_name = "t10k-images-idx3-ubyte";
	char *labels_name = "t10k-labels-idx1-ubyte";

	FILE *images = fopen(images_name,"r");
	FILE *labels = fopen(labels_name,"r");

	Image *img = malloc(sizeof(Image));
	
	Label label = 0;

	// move the layer's outputs to our new image object
	int i=0; // reusable iterable
	for (i; i<10; i++){
		l.cells[i].inputs = img->pixels;
	}

	fseek(images,16,SEEK_SET);
	fseek(labels,8,SEEK_SET);

	double correct = 0;
	double incorrect = 0;

	while (1){
		if (!getNextImage(img,images)) break;
		if (!getNextLabel(&label,labels)) break;
		
		// update cell outputs
		for (i=0; i<10; i++){
			updateOutput(&(l.cells[i]));
		}
	
		// find our machine's answer
		double max = l.cells[0].output;
		int maxPlace = 0;
		for (i=1; i<10; i++){
			if (l.cells[i].output > max){
				maxPlace = i;
				max = l.cells[i].output;
			}
		}

		if (maxPlace == label) correct++;
		else incorrect++;

		printf("Thought answer was %d ; it was %d    ::    Correct: %f, Wrong: %f, ratio: %f\n",maxPlace,label,correct,incorrect,correct/(correct+incorrect));

		
			
	}

}





Layer trainLayer(int print){
	// open training files

	char *images_name = TRAINING;
	char *labels_name = LABELS;

	FILE *images = fopen(images_name,"r");
	FILE *labels = fopen(labels_name,"r");

	Image *img = malloc(sizeof(Image));
	Label label = 0;

	Layer l;

	int i=0; // reusable iterable
	for (i; i<10; i++){
		l.cells[i] = *newPerceptron(img->pixels);
	}


	// move to first pixel and label (ignore headers)
	fseek(images,16,SEEK_SET);
	fseek(labels,8,SEEK_SET);

	double correct = 0;
	double incorrect = 0;

	while (1){
		if (!getNextImage(img,images)) break;
		if (!getNextLabel(&label,labels)) break;

		if (print){
			printImage(img,1);
			printf("Label: %d\n",label);
			getchar();
		}


		// these need allocated inside loop
		Answer *machineAnswer = malloc(sizeof(Answer));
		Answer *correctAnswer = malloc(sizeof(Answer));

		// reset these because apparently malloc hates me
		for (i=0; i<10; i++){
			correctAnswer->vect[i] = 0;
			machineAnswer->vect[i] = 0;
		}

		// set correct answer
		correctAnswer->vect[label] = 1;

		// update each perceptron's output
		for (i=0; i<10; i++){
			updateOutput(&(l.cells[i]));
		}

		double max = l.cells[0].output;
		int maxPlace = 0;
		for (i=1; i<10; i++){
			if (l.cells[i].output > max){
				maxPlace = i;
				max = l.cells[i].output;
			}
		}

		if (maxPlace == label) correct++;
		else incorrect++;	
		machineAnswer->vect[maxPlace] = 1; // set our machine's answer

		// find error
		// update weight of each perceptron as we do so
		for (i=0; i<10; i++){
			//error->vect[i] = correctAnswer->vect[i] - machineAnswer->vect[i];
			updateWeight(&(l.cells[i]),correctAnswer->vect[i] - machineAnswer->vect[i]);
		}

		printf("Thought answer was %d ; it was %d    ::    Correct: %f, Wrong: %f, ratio: %f\n",maxPlace,label,correct,incorrect,correct/(correct+incorrect));

		// reset answers
		free(machineAnswer);
		free(correctAnswer);

	}
	return l;

}

int main(int argc, char **argv){
	srand(time(NULL));


	int simpleFlag = 0;
	if (argc > 1){
		sscanf(argv[1],"%d",&simpleFlag);
	}


	Layer l = trainLayer(simpleFlag);

	printf("Done training.\n Here's what each number looks like (hopefully) : \n");

	int i=0;
	for (i; i < 10; i++){
		Image q = weightsToImage(l.cells[i]);
		printf("%d: \n",i);
		printImage(&q,0);
		printf("\n\n");
	}

	getchar();

	testLayer(l);
}
