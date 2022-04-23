// Do not edit this file. Edit and run generate_inline_config.py, instead.
// This file is to be included in exactly one .cpp file.

const char* param_config_data = 
  "[\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"Initialize\",\n"
  "    \"display name\":\"Initialize\",\n"
  "    \"description\":\"(De)Initialize the Hardware\",\n"
  "    \"data type\": \"enum\",\n"
  "    \"read-only\":false,\n"
  "    \"default\":\"OFF\",\n"
  "    \"persistent\":false,\n"
  "    \"unit\":\"\",\n"
  "    \"options\":{\n"
  "      \"OFF\":0,\n"
  "      \"ON\":1\n"
  "    },\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"DLD_INIT\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"ConfigFile\",\n"
  "    \"display name\":\"Ini File\",\n"
  "    \"description\":\"Config/ini file for TDC\",\n"
  "    \"data type\": \"string\",\n"
  "    \"read-only\":false,\n"
  "    \"default\":\"tdc_gpx3.ini\",\n"
  "    \"persistent\":true,\n"
  "    \"unit\":\"\",\n"
  "    \"maxlen\":2048,\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"DLD_CFGFILE\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"StatusMessage\",\n"
  "    \"display name\":\"Status\",\n"
  "    \"description\":\"Last Status message\",\n"
  "    \"data type\": \"string\",\n"
  "    \"read-only\":true,\n"
  "    \"default\":\"\",\n"
  "    \"unit\":\"\",\n"
  "    \"maxlen\":256,\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"Exposure\",\n"
  "    \"display name\":\"Exposure\",\n"
  "    \"description\":\"Duration of a measurement\",\n"
  "    \"data type\":\"float64\",\n"
  "    \"read-only\":false,\n"
  "    \"default\":\"1.0\",\n"
  "    \"persistent\":true,\n"
  "    \"unit\":\"s\",\n"
  "    \"precision\":3,\n"
  "    \"range\":{\n"
  "      \"min\":0.0,\n"
  "      \"max\":2e6\n"
  "    },\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"Acquire\",\n"
  "    \"display name\":\"Acquire\",\n"
  "    \"description\":\"Start/stop acquisition\",\n"
  "    \"data type\":\"enum\",\n"
  "    \"read-only\":false,    \n"
  "    \"default\":\"OFF\",\n"
  "    \"persistent\":false,\n"
  "    \"unit\":\"\",\n"
  "    \"options\":{\n"
  "      \"OFF\":0,\n"
  "      \"ON\":1\n"
  "    },\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"ImageMode\",\n"
  "    \"display name\":\"Image mode\",\n"
  "    \"description\":\"Single img or sequence\",\n"
  "    \"data type\": \"enum\",\n"
  "    \"read-only\":false,\n"
  "    \"default\":\"Single\",\n"
  "    \"persistent\":false,\n"
  "    \"unit\":\"\",\n"
  "    \"options\":{\n"
  "      \"Single\":0,\n"
  "      \"Multiple\":1,\n"
  "      \"Continous\":2\n"
  "    },\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"NumImages\",\n"
  "    \"display name\":\"Number of images\",\n"
  "    \"description\":\"Number of images\",\n"
  "    \"data type\":\"int32\",\n"
  "    \"read-only\":false,\n"
  "    \"default\":\"10\",\n"
  "    \"persistent\":true,\n"
  "    \"unit\":\"\",\n"
  "    \"range\":{\n"
  "      \"min\":1,\n"
  "      \"max\":1e6\n"
  "    },\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"DataType\",\n"
  "    \"display name\":\"Data type\",\n"
  "    \"description\":\"data type of images\",\n"
  "    \"data type\": \"enum\",\n"
  "    \"read-only\":false,\n"
  "    \"default\":\"UInt32\",\n"
  "    \"persistent\":false,\n"
  "    \"unit\":\"\",\n"
  "    \"options\":{\n"
  "      \"Int8\":0,\n"
  "      \"UInt8\":1,\n"
  "      \"Int16\":2,\n"
  "      \"UInt16\":3,\n"
  "      \"Int32\":4,\n"
  "      \"UInt32\":5,\n"
  "      \"Float32\":6,\n"
  "      \"Float64\":7\n"
  "    },\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"\"\n"
  "    }\n"
  "  },\n"
  "  {\n"
  "    \"node\":\"parameter\",\n"
  "    \"name\":\"Ratemeter\",\n"
  "    \"display name\":\"rate meter\",\n"
  "    \"description\":\"Displays rates of individual TDC channels and combined DLD events\",\n"
  "    \"data type\":\"array1d\",\n"
  "    \"element data type\":\"i32\",\n"
  "    \"maxlen\":70,\n"
  "    \"read-only\":true,\n"
  "    \"default\":\"\",\n"
  "    \"unit\":\"events/sec\",\n"
  "    \"epicsprops\":{\n"
  "      \"asynportname\":\"DLD_RATEMETER\"\n"
  "    }\n"
  "  }\n"
  "]\n";
