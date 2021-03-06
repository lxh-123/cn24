/*
 * This file is part of the CN24 semantic segmentation software,
 * copyright (C) 2015 Clemens-Alexander Brust (ikosa dot de at gmail dot com).
 *
 * For licensing information, see the LICENSE file included with this project.
 */

#include <string>
#include <sstream>

#ifdef BUILD_BOOST
#include <boost/regex.hpp>
#else
#include <regex>
#endif

#include "ConvolutionLayer.h"
#include "NonLinearityLayer.h"
#include "MaxPoolingLayer.h"
#include "AdvancedMaxPoolingLayer.h"
#include "GradientAccumulationLayer.h"
#include "ResizeLayer.h"
#include "HMaxActivationFunction.h"
#include "LayerFactory.h"

namespace Conv {
bool LayerFactory::IsValidDescriptor(std::string descriptor) {
  const std::string valid_descriptor_regex =
    "^[a-z]+(\\("
    "("
    "[a-z]+=[a-zA-Z0-9\\.]+"
    "( [a-z]+=[a-zA-Z0-9\\.]+)*"
    ")?"
    "\\))?$";
#ifdef BUILD_BOOST
  bool valid = boost::regex_match(descriptor, boost::regex(valid_descriptor_regex,boost::regex::extended));
#else
  bool valid = std::regex_match(descriptor, std::regex(valid_descriptor_regex,std::regex::extended));
#endif
  return valid;
}
  
std::string LayerFactory::ExtractConfiguration(std::string descriptor) {
  const std::string config_regex = "[a-z]+\\((.+)\\)";
#ifdef BUILD_BOOST
  boost::smatch config_match;
  bool has_nonempty_configuration = boost::regex_match(descriptor, config_match, boost::regex(config_regex, boost::regex::extended));
#else
  std::smatch config_match;
  bool has_nonempty_configuration = std::regex_match(descriptor, config_match, std::regex(config_regex, std::regex::extended));
#endif
  if(has_nonempty_configuration && config_match.size() == 2) {
    return config_match[1];
  } else {
    return "";
  }
}
  
std::string LayerFactory::ExtractLayerType(std::string descriptor) {
  std::string layertype_regex = "([a-z]+)(\\(.*\\))?";
#ifdef BUILD_BOOST
  boost::smatch config_match;
  bool has_layertype = boost::regex_match(descriptor, config_match, boost::regex(layertype_regex, boost::regex::extended));
#else
  std::smatch config_match;
  bool has_layertype = std::regex_match(descriptor, config_match, std::regex(layertype_regex, std::regex::extended));
#endif
  if(has_layertype && config_match.size() > 1) {
    return config_match[1];
  } else {
    return "";
  }
}
  
#define CONV_LAYER_TYPE(ltype,lclass) else if (layertype.compare(ltype) == 0) { \
layer = new lclass (configuration) ; \
}
  
Layer* LayerFactory::ConstructLayer(std::string descriptor) {
  if (!IsValidDescriptor(descriptor))
    return nullptr;
  std::string configuration = ExtractConfiguration(descriptor);
  std::string layertype = ExtractLayerType(descriptor);
  
  Layer* layer = nullptr;
  if(layertype.length() == 0) {
    // Leave layer a nullptr
  }
  CONV_LAYER_TYPE("convolution", ConvolutionLayer)
  CONV_LAYER_TYPE("maxpooling", MaxPoolingLayer)
  CONV_LAYER_TYPE("amaxpooling", AdvancedMaxPoolingLayer)
  CONV_LAYER_TYPE("tanh", TanhLayer)
  CONV_LAYER_TYPE("sigm", SigmoidLayer)
  CONV_LAYER_TYPE("relu", ReLULayer)
  CONV_LAYER_TYPE("gradientaccumulation", GradientAccumulationLayer)
  CONV_LAYER_TYPE("resize", ResizeLayer)
  CONV_LAYER_TYPE("hmax", HMaxActivationFunction);
  
  return layer;
}
  
std::string LayerFactory::InjectSeed(std::string descriptor, unsigned int seed) {
  if(IsValidDescriptor(descriptor)) {
    const std::string has_seed_regex = ".*seed=[0-9]+.*";
    const std::string new_seed_regex = "seed=([0-9])+";
    std::string configuration = ExtractConfiguration(descriptor);
    std::string layertype = ExtractLayerType(descriptor);
    
    std::stringstream seed_ss;
    seed_ss << "seed=" << seed;
    
#ifdef BUILD_BOOST
    bool already_has_seed = boost::regex_match(configuration, boost::regex(has_seed_regex, boost::regex::extended));
#else
    bool already_has_seed = std::regex_match(configuration, std::regex(has_seed_regex, std::regex::extended));
#endif
    if(already_has_seed) {
#ifdef BUILD_BOOST
      std::string new_descriptor = boost::regex_replace(descriptor, boost::regex(new_seed_regex, boost::regex::extended), seed_ss.str());
#else
      std::string new_descriptor = std::regex_replace(descriptor, std::regex(new_seed_regex, std::regex::extended), seed_ss.str());
#endif
      return new_descriptor;
    } else {
      std::stringstream new_descriptor_ss;
      new_descriptor_ss << layertype << "(";
      if(configuration.length() > 0) {
        new_descriptor_ss << configuration << " ";
      }
      new_descriptor_ss << seed_ss.str() << ")";
      std::string new_descriptor = new_descriptor_ss.str();
      return new_descriptor;
    }
  } else {
    return descriptor;
  }
}
  
}