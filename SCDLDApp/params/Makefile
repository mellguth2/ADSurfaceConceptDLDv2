all: glue inline_config db_template

glue: ../src_dldAppLib/glue.hpp
inline_config: ../src_dldAppLib/dldApp_inline_config.hpp
db_template: ../Db/dldDetectorv2.template

../src_dldAppLib/glue.hpp: parameters.json
	bash -c "cd ../src_dldAppLib/generate_glue; ./generate_glue.py"

../src_dldAppLib/dldApp_inline_config.hpp: parameters.json
	bash -c "cd ../src_dldAppLib/generate_inline_config; ./generate_inline_config.py"

../Db/dldDetectorv2.template: parameters.json
	bash -c "cd ../Db/generate_db_template; ./generate_db_template.py"

install: all
clean:
