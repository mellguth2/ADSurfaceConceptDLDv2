parameters.json configures which parameters the libdldApp understands.

After editing this file, the verify script can be run to check JSON conformance
and JSON *schema* conformance.

When this file is updated, some code generators should be run,
(1) for glue.hpp and dldApp_inline.config affecting the library
(2) for dldDetectorv2.template affecting the EPICS areaDetector driver
This is done by the Makefile.

