#ifndef __READ_CUSTOM_NETWORK_PARAMS__
#define __READ_CUSTOM_NETWORK_PARAMS__

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>


typedef double (*function_double_to_double)(double&);

struct trained_network_layer {
  std::string func_name;
  function_double_to_double func;
  std::vector<double> bias;
  std::vector<std::vector<double> > weights;
};

struct trained_network {
  std::vector<trained_network_layer> layers;
  double min_norm_out, max_norm_out, min_out, max_out, tolerance;
  std::vector<double> min_norm_in, max_norm_in;
};


void read_one_line(std::ifstream &file, std::stringstream &ss) {
  std::string line;
  ss.clear(); // remove the error status (e.g. end of buffer) to be able to continue;
  while(getline(file, line)) {
    if(!line.size() || line[0] == '#')
      continue;
    ss << line;
    return;
  }
}

double __purelin(double &x) {
  return x;
}
double __tansig(double &x) {
  return 2.0 / (1.0 + exp(-2.0*x) ) - 1.0;
}

function_double_to_double getFunc(std::string name) {
  if(name == "purelin" || name == "none" || name == "lin")
    return &__purelin;
  else if(name == "tansig")
    return &__tansig;
  else {
    std::cerr << "function " << name << " not implemented !" << std::endl;
    return 0;
  }
}

trained_network bvnet_fill_trained_network(std::string filename) {
  std::ifstream file(filename.c_str(), std::ios::in);
  if(!file)
    std::cerr << "file " << filename << " cannot be opened !" << std::endl;

  trained_network n;
  std::vector<int> layers_size;

  // read first line
  std::string line, field_str;
  unsigned int field_int;
  double field_dbl, field_dbl2, field_dbl3;
  std::stringstream ss;
  read_one_line(file, ss);
  while(ss >> field_str >> field_int) {
    trained_network_layer l;
    l.func_name = field_str;
    l.func = getFunc(field_str);
    layers_size.push_back(field_int);
    n.layers.push_back(l);
  }
  //printf("%d, %s, %d, %d\n", n.layers.size(), n.layers[0].func_name, layers_size.size(), layers_size[0]);

  // read min/max for norm inputs
  read_one_line(file, ss);
  while(ss >> field_dbl >> field_dbl2) {
    n.min_norm_in.push_back(field_dbl);
    n.max_norm_in.push_back(field_dbl2);
  }

  // read layers bias/weights infos
  int old_nb = -1;
  for(unsigned int iLayer = 0; iLayer < layers_size.size(); ++iLayer) {
    for(int iNeuron = 0; iNeuron < layers_size[iLayer]; ++iNeuron) {
      read_one_line(file, ss);
      ss >> field_dbl;
      n.layers[iLayer].bias.push_back(field_dbl);
      std::vector<double> w;
      while(ss >> field_dbl)
        w.push_back(field_dbl);
      n.layers[iLayer].weights.push_back(w);
    }
    // several checks to "ensure" conformity
    if(!n.layers[iLayer].weights.size())
      std::cerr << "no weight found !" << std::endl;
    int nbinput = n.layers[iLayer].weights[0].size();
    if(old_nb > 0 && old_nb != nbinput)
      std::cerr << "nb inputs / nb neurons doesnt match between layers ! (" << old_nb
                << ", " << nbinput << ")" << std::endl;
    int nblay = (int)n.layers[iLayer].weights.size();
    if(nblay != (int)n.layers[iLayer].bias.size())
      std::cerr << "nb bias / nb weigh layers doenst match ! (not possible?)" << std::endl;
    for(int i = 1; i < nblay; ++i)
      if((int)n.layers[iLayer].weights[i].size() != nbinput)
        std::cerr << "nb of weights not coherent inside a same layer" << std::endl;
    old_nb = nblay;
  }


  // read min/max for norm output
  read_one_line(file, ss);
  ss >> field_dbl >> field_dbl2;
  n.min_norm_out = field_dbl;
  n.max_norm_out = field_dbl2;

  // read min/max/tolerance for output
  read_one_line(file, ss);
  ss >> field_dbl >> field_dbl2 >> field_dbl3;
  n.min_out = field_dbl;
  n.max_out = field_dbl2;
  n.tolerance = field_dbl3;

  file.close();

  return n;
}


#endif
