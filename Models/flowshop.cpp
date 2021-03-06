#include "flowshop.h"

flowshop::flowshop(string fileName)
{
	this->directory = "";
	this->fileName = fileName;
	readInstance();
}

flowshop::flowshop(string directory, string fileName)
{
	this->directory = directory;
	this->fileName = fileName;
	readInstance();
}

void flowshop::setupModel()
{
	cout << "---------------Running " << this->fileName << endl << endl << endl;
	GRBEnv env = GRBEnv(true);
	env.start();

	try {
		//env = new GRBEnv();
		GRBModel model = GRBModel(env);
		model.set(GRB_StringAttr_ModelName, "flowshop_" + fileName);

		varX(model);
		varS(model);
		varC(model);
		fo(model);
		c1(model);
		c2(model);
		c3(model);
		c4(model);
		c5(model);
		c6(model);
		c7(model);

		model.write("teste.lp");

		model.getEnv().set(GRB_DoubleParam_TimeLimit, TMAX);

		model.optimize();

		utilities::processSolution(model);
		getSolution(model);
		model.write("teste.sol");
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
}

void flowshop::getSolutionFull(GRBModel &model)
{
	string dir = directory + "output/";
	string fn = model.get(GRB_StringAttr_ModelName);
	//model.write(directory + fn + ".sol");
	fstream output(dir + fn, ios::out | ios::trunc);
	if (output.is_open() == false) {
		cout << "Error opening output file " << fn << endl;
		cout << "On directory " << dir << endl;
		exit(1);
	}
	// writing variables in a format for easy ploting
	output << "# x " << x.size() << " " << x.at(0).size() << endl;
	for (int i = 0; i < x.size(); i++) {
		for (int j = 0; j < x.at(i).size(); j++) {
			auto temp = model.getVarByName("x(" + to_string(i) + "," + to_string(j) + ")").get(GRB_DoubleAttr_X);
			if (temp == -0) { // i dont know why some values are beeing -0
				temp *= -1;
			}
			output << temp << " ";
		}
		output << endl;
	}
	output << "# s " << s.size() << " " << s.at(0).size() << endl;
	for (int i = 0; i < s.size(); i++) {
		for (int j = 0; j < s.at(i).size(); j++) {
			auto temp = model.getVarByName("s(" + to_string(i) + "," + to_string(j) + ")").get(GRB_DoubleAttr_X);
			if (temp == -0) { // i dont know why some values are beeing -0
				temp *= -1;
			}
			output << temp << " ";
		}
		output << endl;
	}
	output << "# c 1" << endl;
	output << model.getVarByName("c").get(GRB_DoubleAttr_X) << endl;
	output.close();
}

void flowshop::getSolution(GRBModel & model)
{
	string dir = directory + "output/";
	string fn = model.get(GRB_StringAttr_ModelName);
	//model.write(directory + fn + ".sol");
	fstream output(dir + fn, ios::out | ios::trunc);
	if (output.is_open() == false) {
		cout << "Error opening output file " << fn << endl;
		cout << "On directory " << dir << endl;
		exit(1);
	}
	// write fo, gap and execution time
	output << model.get(GRB_DoubleAttr_ObjVal) << " " << model.get(GRB_DoubleAttr_MIPGap) * 100 << " " << model.get(GRB_DoubleAttr_Runtime) << endl;
	// writing variables in a format for easy ploting
	for (int i = 0; i < x.size(); i++) {
		for (int j = 0; j < x.at(i).size(); j++) {
			auto temp = model.getVarByName("x(" + to_string(i) + "," + to_string(j) + ")").get(GRB_DoubleAttr_X);
			if (temp == 1) {
				output << j << " ";
			}
		}
	}
	output.close();
}

void flowshop::printData()
{
	cout << numTasks << " " << numMachines << endl;
	cout << "p: \n";
	for (auto i : p) {
		for (auto j : i) {
			cout << j << " ";
		}
		cout << endl;
	}
}


flowshop::~flowshop()
{
}

void flowshop::readInstance()
{
	fstream file(directory + fileName, ios::in);
	if (file.is_open() == false) {
		cout << "Error opening file " << fileName << endl;
		exit(1);
	}
	int counter{ 1 };
	string line;
	while (getline(file, line)) {
		auto tokens = strf::split(line, ' ');
		if (counter == 1) {
			// read number of tasks
			numTasks = stoi(tokens.at(tokens.size() - 1));
		}
		else if (counter == 2) {
			// read number of machines
			numMachines = stoi(tokens.at(tokens.size() - 1));
		}
		else if (counter >= 4) {
			// reading Cij, the time of task i in the machine j
			vector<int> aux;
			for (auto tk : tokens) {
				aux.push_back(stoi(tk));
			}
			p.push_back(aux);
		}
		counter++;
	}
	file.close();
}

void flowshop::varX(GRBModel &model)
{
	// resize x
	x.resize(numTasks);
	for (int i = 0; i < numTasks; i++) {
		x.at(i).resize(numTasks);
	}
	// add x to model
	for (int i = 0; i < numTasks; i++) {
		for (int j = 0; j < numTasks; j++) {
			x.at(i).at(j) = model.addVar(0, 1, 1, GRB_BINARY, "x(" + to_string(i) + "," + to_string(j) + ")");
		}
	}
}

void flowshop::varS(GRBModel &model)
{
	// resize s
	s.resize(numMachines);
	for (int i = 0; i < numMachines; i++) {
		s.at(i).resize(numTasks);
	}
	// add x to model
	for (int i = 0; i < numMachines; i++) {
		for (int j = 0; j < numTasks; j++) {
			s.at(i).at(j) = model.addVar(0, GRB_INFINITY, 1, GRB_CONTINUOUS, "s(" + to_string(i) + "," + to_string(j) + ")");
		}
	}
}

void flowshop::varC(GRBModel &model)
{
	c = model.addVar(0, GRB_INFINITY, 1, GRB_CONTINUOUS, "c");
}

void flowshop::fo(GRBModel &model)
{
	GRBLinExpr fo = 0;
	fo += s.at(numMachines - 1).at(numTasks - 1);
	for (int i = 0; i < numTasks; i++) {
		fo += p.at(i).at(numMachines - 1) * x.at(i).at(numTasks - 1);
	}
	model.setObjective(fo, GRB_MINIMIZE);
	model.update();
}

// each task i must be associated to a unique position j
void flowshop::c1(GRBModel &model)
{
	for (int i = 0; i < numTasks; i++) {
		GRBLinExpr c1 = 0;
		for (int j = 0; j < numTasks; j++) {
			c1 += x.at(i).at(j);
		}
		model.addConstr(c1 == 1, "c(" + to_string(i) + ")");
	}
	model.update();
}

// each position j must be associated to a unique task i
void flowshop::c2(GRBModel &model)
{
	for (int j = 0; j < numTasks; j++) {
		GRBLinExpr c2 = 0;
		for (int i = 0; i < numTasks; i++) {
			c2 += x.at(i).at(j);
		}
		model.addConstr(c2 == 1, "c(" + to_string(j) + ")");
	}
	model.update();
}

// task 0 must begin at time 0 in the machine 0
void flowshop::c3(GRBModel &model)
{
	model.addConstr(s.at(0).at(0) == 0, "c3");
}

// calculates processing times for every tasks in machine 0
void flowshop::c4(GRBModel &model)
{
	for (int j = 0; j < numTasks - 1; j++) {
		GRBLinExpr c4 = 0;
		c4 += s.at(0).at(j);
		for (int i = 0; i < numTasks; i++) {
			c4 += p.at(i).at(0) * x.at(i).at(j);
		}
		model.addConstr(c4 == s.at(0).at(j + 1), "c(" + to_string(j) + ")");
	}
	model.update();
}

// calculates processing times of the first taks in machines 2 to m-1
void flowshop::c5(GRBModel &model)
{
	for (int k = 0; k < numMachines - 1; k++) {
		GRBLinExpr c5 = 0;
		c5 += s.at(k).at(0); // c5 += s.at(k).at(1);
		for (int i = 0; i < numTasks; i++) {
			c5 += p.at(i).at(k) * x.at(i).at(0);
		}
		model.addConstr(c5 == s.at(k + 1).at(0), "c5(" + to_string(k) + ")"); // model.addConstr(c5 == s.at(k + 1).at(1), "c5(" + to_string(k) + ")");
	}
	model.update();
}

// the task in the position j >= 2 cant be started in the next machine k+1 before be finished on your current machine
void flowshop::c6(GRBModel &model)
{
	for (int j = 1; j < numTasks; j++) {
		for (int k = 0; k < numMachines - 1; k++) {
			GRBLinExpr c6 = 0;
			c6 += s.at(k).at(j);
			for (int i = 0; i < numTasks; i++) {
				c6 += p.at(i).at(k) * x.at(i).at(j);
			}
			model.addConstr(c6 <= s.at(k+1).at(j), "c6(" + to_string(j) + "," + to_string(k) + ")");
		}
	}
	model.update();
}

// the task in position j+1 will begin to be proccessed on machine k after the task of position j finish your processing in the same machine
void flowshop::c7(GRBModel &model)
{
	for (int j = 0; j < numTasks - 1; j++) {
		for (int k = 1; k < numMachines; k++) {
			GRBLinExpr c7 = 0;
			c7 += s.at(k).at(j);
			for (int i = 0; i < numTasks; i++) {
				c7 += p.at(i).at(k) * x.at(i).at(j);
			}
			model.addConstr(c7 <= s.at(k).at(j + 1), "c7(" + to_string(j) + "," + to_string(k) + ")");
		}
	}
	model.update();
}
