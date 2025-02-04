#include "ArtificialIntelligence.h"

ArtificialIntelligence::ArtificialIntelligence(){
	accuracy = 0.8;
	noise = 0.1;
 	discount = 1.0;
	stdReward = 0;
	generationsRL = 0;

	impartialState = -15;
	maxReward = 1;
	minReward = -0.3;
	wallReward = -0.1;
	stuckReward = -200;
	countQ = 0;
	countRG = 0;

	//DQN constants

	epoch = 0;

	actualAction = -1;
 
	epsilon =1.0f;
	gamma = 0.95;

	batch = 55;
	buffer = 10000;
	priorBuffer = 5;
	countBuffer = 0;
	isTerminal = false;

	followPolicy = false;

	numEpochs = 30000;
	actReward = 0.0;
	takenAct = 0;
	numCopies = 0;
	policyValue= 0;

	nShowError = 30;
	netError = 0;

	constructNet();
	//std::ifstream input("net_ostream-4.txt");
	//net.load(input);
}

void ArtificialIntelligence::reinitMapProp(){
	takenAct = 0;
}

void ArtificialIntelligence::setFollowPolicy(bool followPolicy){
	this->followPolicy = followPolicy;
}

void ArtificialIntelligence::saveNetwork(){
	stringstream errorFile1;
	errorFile1 << "net_ostream-" << numCopies << ".txt";
	std::ofstream file1(errorFile1.str());
	file1 << net;

	file1.close();

	stringstream errorFile2;
	errorFile2 << "PoliciesValue-" << numCopies << ".txt";
	std::ofstream file2(errorFile2.str());
	for(int i = 0; i < listPolicies.size(); i++){
		file2 << listPolicies[i] << ";" << endl;
	}

	file2.close();

	stringstream errorFile;
	errorFile << "ErrorList-" << numCopies << ".txt";
	std::ofstream file3(errorFile.str());
	for(int i = 0; i < listErrors.size(); i++){
		file3 << listErrors[i] << ";" << endl;
	}

	file3.close();
}

void ArtificialIntelligence::constructNet() {
	initNetwork();

	vector<ActivationFunc*> activations;
	ActivationFunc* leakyRelu = new LeakyRelu();
	ActivationFunc* identity = new Identity();
	activations.push_back(leakyRelu);
	activations.push_back(leakyRelu);
	activations.push_back(identity);

	//myNet = NeuralNetwork(topology, activations);
}

void ArtificialIntelligence::initNetwork(){
	float input = (SIZE_WIDTH/SCALE_MAP+2)*(SIZE_DEPTH/SCALE_MAP+2);

	net = network<sequential>();

	switch(numCopies){
		case 0:{
			net << fully_connected_layer<leaky_relu>(input, 100)
			<< fully_connected_layer<leaky_relu>(100, 125)
			<< fully_connected_layer<identity>(125, 4);
		}break;
		case 1:{
			net << fully_connected_layer<leaky_relu>(input, 125)
			<< fully_connected_layer<leaky_relu>(125, 125)
			<< fully_connected_layer<identity>(125, 4);
		}break;
		case 2:{
			net << fully_connected_layer<leaky_relu>(input, 100)
			<< fully_connected_layer<leaky_relu>(100, 100)
			<< fully_connected_layer<identity>(100, 4);
		}break;
		case 3:{
			net << fully_connected_layer<leaky_relu>(input, 150)
			<< fully_connected_layer<leaky_relu>(150, 200)
			<< fully_connected_layer<identity>(200, 4);
		}break;
		case 4:{
			net << fully_connected_layer<leaky_relu>(input, 200)
			<< fully_connected_layer<leaky_relu>(200, 150)
			<< fully_connected_layer<identity>(150, 4);
		}break;
		case 5:{
			net << fully_connected_layer<leaky_relu>(input, 174)
			<< fully_connected_layer<identity>(174, 4);
		}break;
		case 6:{
			net << fully_connected_layer<leaky_relu>(input, 120)
			<< fully_connected_layer<identity>(120, 4);
		}break;
		case 7:{
			net << fully_connected_layer<leaky_relu>(input, 100)
			<< fully_connected_layer<leaky_relu>(100, 4);
		}break;
		case 8:{
			net << fully_connected_layer<leaky_relu>(input, 100)
			<< fully_connected_layer<leaky_relu>(100, 150)
			<< fully_connected_layer<identity>(150, 4);
		}break;
		case 9:{
			net << fully_connected_layer<leaky_relu>(input, 200)
			<< fully_connected_layer<leaky_relu>(200, 100)
			<< fully_connected_layer<identity>(100, 4);
		}break;
		case 10:{
			net << fully_connected_layer<leaky_relu>(input, 150)
			<< fully_connected_layer<leaky_relu>(150, 125)
			<< fully_connected_layer<identity>(125, 4);
		}break;
		case 11:{
			net << fully_connected_layer<leaky_relu>(input, 165)
			<< fully_connected_layer<identity>(165, 4);
		}break;
		case 12:{
			net << fully_connected_layer<leaky_relu>(input, 200)
			<< fully_connected_layer<identity>(200, 4);
		}break;
		case 13:{
			net << fully_connected_layer<leaky_relu>(input, 80)
			<< fully_connected_layer<leaky_relu>(80, 165)
			<< fully_connected_layer<identity>(165, 4);
		}break;

	}
	/*
		net << convolutional_layer<leaky_relu>((SIZE_WIDTH/SCALE_MAP+2)*4,(SIZE_DEPTH/SCALE_MAP+2),3,1,10, padding::valid,true,1,1)
			<< convolutional_layer<leaky_relu>(38,8,3,10,10,padding::valid,true,1,1)
			<< convolutional_layer<leaky_relu>(36,6,3,10,10,padding::valid,true,2,2)
			<< fully_connected_layer<identity>(340, 4);
		

		net << fully_connected_layer<leaky_relu>(input, 165)
			<< fully_connected_layer<leaky_relu>(165, 165)
			<< fully_connected_layer<identity>(165, 4);
	*/

	net.init_weight();
}

void ArtificialIntelligence::setMap(Map mapVision, float reward, bool isTerminal){
	cout << "EPOCH: " << epoch << endl;
	cout << "Taken: " << takenAct << endl;

	if(takenAct > 0){
		concatPrevMapVisions.clear();
		concatPrevMapVisions.insert(concatPrevMapVisions.end(),concatInputMapVisions.begin(),concatInputMapVisions.end());
		this->isTerminal = isTerminal;
		this->prevMapVision = vec_t(this->imageInput);
		actReward = reward;
	}
	cout << "Reward: " << actReward << endl;

	//Use to increase window and see what is happening
	/*Mat imVision;
	cv::resize(mapVision,imVision,cv::Size(150,150));
	namedWindow( "Display window", WINDOW_AUTOSIZE );
	
	imshow("Display window", imVision);
	waitKey(1);*/
	this->mapVision = mapVision;
	
	processRL();

	epoch++;
	takenAct++;

	cout  << endl << endl;

}

vector<GameMemory> ArtificialIntelligence::selectRandomBatch(){
	vector<GameMemory> batchSelect;

	for(int i = 0; i < batch; i++){
		
		float percPrior = ((rand()%100))/100.f;
		if(percPrior < 0.1){
			int index = rand()%((int)priorReplay.size());
			batchSelect.push_back(priorReplay.at(index)); 
		}
		else{
			int index = rand()%buffer;
			batchSelect.push_back(replay.at(index)); 
		}
	}

	return batchSelect;
}

void ArtificialIntelligence::processRL(){
	imageInput = processImage();

	concatInputMapVisions.clear();
	if(takenAct == 0){
		concatInputMapVisions.insert(concatInputMapVisions.end(),imageInput.begin(),imageInput.end());
		//concatInputMapVisions.insert(concatInputMapVisions.end(),imageInput.begin(),imageInput.end());
	 	//concatInputMapVisions.insert(concatInputMapVisions.end(),imageInput.begin(),imageInput.end());
		//concatInputMapVisions.insert(concatInputMapVisions.end(),imageInput.begin(),imageInput.end());
	}else{
		concatInputMapVisions.insert(concatInputMapVisions.end(),concatPrevMapVisions.begin()+imageInput.size(),concatPrevMapVisions.end());
		concatInputMapVisions.insert(concatInputMapVisions.end(),imageInput.begin(),imageInput.end());
	}
		
	vec_t output = net.predict(concatInputMapVisions);


	for(int t = 0; t < output.size();t++){
		string strAct = "";
		switch(t){
			case 0: strAct = "v"; break;
			case 1: strAct = ">"; break;
			case 2: strAct = "^"; break;
			case 3: strAct = "<"; break;
		}

		cout << "Outputs " << strAct << " "<< t << ":" << output.at(t) << endl;
	}

	if(followPolicy){
		actualAction = calculateMaxOutput(output);
		return;
	}
	
	int prevAction = actualAction;
	actualAction = calculateActualState(output);

	if(takenAct > 150) policyValue += actReward;
	
	if(takenAct == 0){
		policyValue = 0;
		if(replay.size() >= buffer){
			listPolicies.push_back(policyValue);
			cout << "Policy saved" << endl;
		}
		return;
	} 
	if(replay.size() >= buffer ) policyValue += output.at(actualAction);

	/*cout << "Mapa antigo:" << endl;
	showState(concatPrevMapVisions);

	cout << "Mapa atual:" << endl;
	showState(concatInputMapVisions);*/

	bool sameState = false;
	if(equal(concatInputMapVisions.begin(),concatInputMapVisions.end(),concatPrevMapVisions.begin())){
		cout << "MESMO ESTADO" << endl;
		sameState=true;
	} 
	
	bool memIsTerminal = actReward == stdReward ? false : true; 
	GameMemory gameMemory(concatPrevMapVisions,prevAction, actReward, concatInputMapVisions, memIsTerminal);
	
	if(replay.size() < buffer || priorReplay.size() < 1){

		//replay.push_back(gameMemory);
		if(actReward < maxReward){ 
			replay.push_back(gameMemory);
		}else {
			priorReplay.push_back(gameMemory);
		}
	}else{
		vector<GameMemory> miniBatch = selectRandomBatch();

		int countR = 0;
		vector<vec_t> inputs_train;
		vector<vec_t> outputs_train;
		for(int i = 0; i < batch; i++){
			GameMemory mem = miniBatch.at(i);

			vec_t newQ = net.predict(mem.newMapVision);
			vec_t oldQ = net.predict(mem.oldMapVision);
			float maxQ = calculateMaxOutput(newQ);
			//maxQ = calculateActualState(newQ);

			vec_t oldOutputs(oldQ);
			float stateValue = 0;
			if(mem.isTerminal){
				stateValue = mem.reward;
			}else{
				stateValue = mem.reward + gamma*newQ.at(maxQ);
			}

			oldOutputs.at(mem.action) = stateValue;

			inputs_train.push_back(mem.oldMapVision);
			outputs_train.push_back(oldOutputs);
		}

		if(nShowError == 30){
			netError = net.get_loss<mse>(inputs_train,outputs_train);
			listErrors.push_back(netError);

			nShowError = 0;
		}else{
			nShowError++;
		}

		cout << "Error: " << netError << endl;

		net.fit<mse>(mptimizer,inputs_train, outputs_train,batch,1);

		if(!sameState){
			if(actReward < maxReward){
				int indexBuffer = rand()%buffer;
				replay.at(indexBuffer) = gameMemory;
			}else{
				if(priorReplay.size() < priorBuffer){
					priorReplay.push_back(gameMemory);
				}else{
					int indexBuffer = rand()%priorBuffer;
					priorReplay.at(indexBuffer) = gameMemory;
				}
			}
		}

		epsilon -= 1.0f/numEpochs;
		if(epsilon < 0.05) epsilon = 0.05;
		cout << "ep: " << epsilon << endl;
		
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		if(epsilon < 0.7){
			cout << "Reiniciou aqui" << endl;
		    epsilon = 1.0;
			saveNetwork();
			constructNet();
			numCopies++;
			listErrors.clear();
			listPolicies.clear();
			replay.clear();
		}
	}
}

vec_t ArtificialIntelligence::processImage(){
	vec_t inputNet;

	/*for(int i= mapVision.size() -1; i >= 0;i--){
        for(int j= 0; j < mapVision.at(0).size();j++){
            cout << mapVision.at(i).at(j) << " ";
        }   
        cout << endl;  
    }*/

	for(int i = 0; i < mapVision.size();i++){
		for(int j = 0; j < mapVision.at(0).size();j++){
			int index = mapVision.at(i).at(j);
			switch(index){
				case 0:{
					inputNet.push_back(1.0f);
					//inputNet.push_back(1.0f);
					//inputNet.push_back(0.75f);
					//inputNet.push_back(0.75f);
				}break;
				case 1:{
					inputNet.push_back(0.8f);
					//inputNet.push_back(0.8);
					//inputNet.push_back(0.5);
					//inputNet.push_back(0.5);
				}break;
				case 2:{
					inputNet.push_back(0.6f);
					//inputNet.push_back(0.9);
					//inputNet.push_back(0.25);
					//inputNet.push_back(0.9);
				}break;
				case 5:{
					inputNet.push_back(0.4f);
					//inputNet.push_back(1.0f);
					//inputNet.push_back(0.3);
					//inputNet.push_back(0.7);
					//inputNet.push_back(0.7);
				}break;
				case 8:{
					inputNet.push_back(0.2f);
					//inputNet.push_back(1.0);
					//inputNet.push_back(0.7);
					//inputNet.push_back(1.0);
				}break;
			}
		}
	}

	return inputNet;
}

void ArtificialIntelligence::showState(vec_t vision){
	int numIterations = vision.size()/imageInput.size();
	int genIndex = 0;

	vector<Map> print;
	for(int w = 0; w < numIterations; w++){
		Map itMap;
		for(int i= mapVision.size() -1; i >= 0;i--){
			vector<float> row;
			for(int j= 0; j < mapVision.at(0).size();j++){
				row.push_back(vision.at(genIndex));
				genIndex++;
			}
			itMap.push_back(row);    
		}
		print.push_back(itMap);
	}

	for(int w = 0; w < numIterations; w++){
		for(int i= mapVision.size() -1; i >= 0;i--){
			for(int j= 0; j < mapVision.at(0).size();j++){
				//float_t index = vision.at(genIndex);
				printf("%.1f ", print.at(w).at(i).at(j));
				genIndex++;
			}   
			cout << endl;  
		}
		cout << endl; 
	}
}

bool ArtificialIntelligence::reachDeepState(){


	return true;
}

int ArtificialIntelligence::calculateMaxOutput(vec_t output){
	float maxAction = -10000;
	int action;
	for(int i = 0; i < output.size();i++){
		//cout << "Values " << i << ":" << output.at(i) << endl;
		if(output.at(i) > maxAction){
			maxAction = output.at(i);
			action = i;
		}
	}

	//cout << "Action : " << action << endl;
	return action;
}

int ArtificialIntelligence::calculateActualState(vec_t output){
	int action = calculateMaxOutput(output);

	float randF = (rand()%100+1)/100.f;

	if(randF < epsilon)
		action = rand()%(4);

	return action;
}

float ArtificialIntelligence::calculateQValue(){
	return 0;
}

void ArtificialIntelligence::showBestResult(){
	//for(int j = gameDepth-1; j >= 0;j--){
	for(int j = 0; j < gameDepth;j++){
		for(int i = 0; i < gameWidth ;i++){
			printf("%.2f\t",mdp.at(j).at(i));
		}
		cout << endl;
	}

	cout << endl << "Policy:" << endl;
	//for(int j = gameDepth-1; j >= 0;j--){
	for(int j = 0; j < gameDepth;j++){
		for(int i = 0; i < gameWidth ;i++){
			cout << policy.at(j).at(i) << "\t";
		}
		cout << endl;
	}
}

Neuron::Neuron(int numOutputs, int nodeId, ActivationFunc* function){
	this->activationFunc = function;
	m_nodeId = nodeId;
	isBias = false;
	for(int i = 0;i<numOutputs;i++){
		m_connections.push_back(Connection());
		m_connections.back().outWeight = randomWeight();
		m_connections.back().totalDelta = 0;
	}
}

void Neuron::setIsBias(bool bias){
	isBias = bias;
}

void Neuron::update(int m){
	double alpha = 0.001;
	double gammaRms = 0.95;

	for(int i = 0;i<m_connections.size();i++){
		double v = alpha*m_connections[i].totalDelta/float(m);
		m_connections[i].weightGrad = gammaRms*m_connections[i].weightGrad + (1-gammaRms)*v*v; 

		m_connections[i].outWeight +=  alpha*v/sqrt(m_connections[i].weightGrad);
		
	}
}

void Neuron::setOutputValue(double val){
	m_output = val;
}

void Neuron::updateDeltaWeights(Layer& prevLayer){
	double alpha = 0.05;

	for(int j = 0; j < prevLayer.size(); j++){
		Neuron& node = prevLayer[j];

		double newDeltaWeight = alpha*node.getOutputValue()*m_delta;

		node.m_connections[m_nodeId].weightGrad = newDeltaWeight;
		node.m_connections[m_nodeId].totalDelta += newDeltaWeight;

		//node.m_connections[m_nodeId].outWeight += newDeltaWeight;
		//node.m_connections[m_nodeId].outWeight += newDeltaWeight;

	}
}

double Neuron::sigmoid(double z){
	double activation = 1/(1+exp(-z));
	return activation;
}

double Neuron::derivateSigmoid(double targetVal){
	double derivate = targetVal*(1-targetVal);
	if(isBias) return 1;
	else return derivate;
}

double Neuron::leakyRelu(double z){
	if(z > 0)
		return z;
	else
		return 0.01*z;
}

double Neuron::derivateLeakyRelu(double z){
	if(z > 0)
		return 1;
	else
		return 0.01;
} 

double Neuron::identity(double z){
	return z;
}

double Neuron::derivateIdentity(){
	return 1;
}

void Neuron::calcOutputDelta(double targetVal){
	m_delta = targetVal - m_output;
}

void Neuron::calcHiddenDelta(Layer& nextLayer){

	double sum = 0;
	for(int k = 0;k <  nextLayer.size()-1; k++){
		sum += m_connections[k].outWeight*nextLayer[k].getDelta();
	}

	m_delta = sum*activationFunc->derivateActivation(m_output);
	if(isBias) m_delta = sum;
}

void Neuron::feedForward(Layer &prevLayer){
	float sum = 0.0;
	//cout << "Layer: " << m_nodeId << endl;
	for(int i = 0; i < prevLayer.size();i++){
		sum += prevLayer[i].getOutputValue()*prevLayer[i].getConnections()[m_nodeId].outWeight;//<----------CORRECT
		//cout << sum << endl;
	}
	//cout << "end:\t " << sum << endl;
	m_hypOut = sum;
	m_output = activationFunc->activation(sum);
	
}

NeuralNetwork::NeuralNetwork(vector<int> topology, vector<ActivationFunc*> activations){
	int numLayers = topology.size();
	for(int i = 0; i < numLayers;i++){
		m_layers.push_back(Layer());
		int numOutputs = 0;
		if(i != numLayers - 1) numOutputs = topology[i+1];
		else numOutputs = 1;

		

		//if(i == numLayers-1) topology.at(i) = topology[i] - 1;

		for(int j = 0; j <= topology[i];j++){
			m_layers.back().push_back(Neuron(numOutputs,j,activations.at(i)));
		}

		m_layers.back().back().setOutputValue(1.0);
		m_layers.back().back().setIsBias(true);

	}
}

void NeuralNetwork::feedForward(vector<double> inputVals){
	assert(inputVals.size() == m_layers[0].size() -1  && "Você colocou um número de entradas maior/menor que o número de nós na camada de entrada!");

	//cout << "Layer:"<< endl;
	for(int i = 0; i < inputVals.size();i++){
		m_layers[0][i].setOutputValue(inputVals[i]);
		//cout << "weight 0:\t" << m_layers[1][0].getConnections()[i].outWeight << endl << "weight 1:\t" << m_layers[1][1].getConnections()[i].outWeight << endl << "weight 2:\t" << m_layers[1][2].getConnections()[i].outWeight << endl;
	}


	for(int i = 1; i < m_layers.size();i++){
		Layer& prevLayer = m_layers[i - 1];
		for(int j = 0; j < m_layers[i].size()-1;j++){
			m_layers[i][j].feedForward(prevLayer);
		}
	}
}

void NeuralNetwork::calcActualError(vector<double> targetVals){
	m_error = 99999;

	int k = m_layers.back().size()- 1;
	assert(targetVals.size() == k && "Você colocou um número de saídas maior/menor que o número de nós na camada de saída!");

	double deltaSquare = 0;
	for(int i = 0; i < k;i++){
		double diff = m_layers.back()[i].getOutputValue() - targetVals[i];
		cout << "saida " << i << ": " << diff << endl;
		deltaSquare += diff*diff;
	}

	deltaSquare /= k;
	m_error = sqrt(deltaSquare);
}

void NeuralNetwork::backProp(vector<double> targetVals){
	int finalLayer = m_layers.size()-1;
	int k = m_layers.back().size()-1;

	assert(targetVals.size() == k && "Você colocou um número de saídas maior/menor que o número de nós na camada de saída!");

	calcActualError(targetVals);

	//for output layer
	for(int node = 0; node < k; node++){
		m_layers.back()[node].calcOutputDelta(targetVals[node]);
	}

	//for hidden layers
	for(int layer = finalLayer - 1; layer > 0; layer--){
		Layer& actLayer = m_layers[layer];
		Layer& nextLayer = m_layers[layer+1];
		for(int i = 0; i < actLayer.size();i++){
			actLayer[i].calcHiddenDelta(nextLayer);
		}
	}

	for(int layer = finalLayer; layer > 0;layer--){
		Layer& actLayer = m_layers[layer];
		Layer& prevLayer = m_layers[layer-1];
		for(int j = 0; j < actLayer.size()-1;j++){
			actLayer[j].updateDeltaWeights(prevLayer);
		}
	}
}

void NeuralNetwork::SGD(int miniBatch){
	for(int layer = 0; layer < m_layers.size();layer++){
		Layer& actLayer = m_layers.at(layer);
		for(int neuron = 0;neuron < actLayer.size();neuron++){
			Neuron& actNeuron = actLayer.at(neuron);
			actNeuron.update(miniBatch);
		}
	}
}

vector<double> NeuralNetwork::getResults(){
	vector<double> results;

	for(int j = 0; j < m_layers.back().size()-1; j++){
		results.push_back(m_layers.back()[j].getOutputValue());
	}
	return results;
}