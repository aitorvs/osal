In the directory ./config some pre-defined configuration files are stored. This
files help the OSAL user to configure the library starting of a first pre-define
configuration close to the one the user want to have. The files are

- config_linux.cfg. This file configures OSAL to be executed in a PC Linux
environment. It has disabled all the debug messages and assert sentences. The
dynamic memory allocation debug is enabled and all the tests are compiled

- config_linux_dbg.cfg. This file configures OSAL to be executed in a PC Linux
environment. It has enabled all the debug messages and assert sentences. The
dynamic memory allocation debug is enabled and all the tests are compiled

- config_leon2_rtems.cfg. This file configures OSAL to be executed in a LEON2 
RTEMS environment. It has disabled all the debug messages and assert sentences. 
The dynamic memory allocation debug is enabled and all the tests are compiled

- config_leon2_rtems_dbg.cfg. This file configures OSAL to be executed in a LEON2
RTEMS environment. It has enabled all the debug messages and assert sentences. 
The dynamic memory allocation debug is enabled and all the tests are compiled

- config_leon2_rasta_rtems.cfg. This file configures OSAL to be executed in a LEON2 
RASTA RTEMS environment. It has disabled all the debug messages and assert sentences. 
The dynamic memory allocation debug is enabled and all the tests are compiled

- config_leon2_rasta_rtems_dbg.cfg. This file configures OSAL to be executed in a LEON2
RASTA RTEMS environment. It has enabled all the debug messages and assert sentences. 
The dynamic memory allocation debug is enabled and all the tests are compiled


To ise any of this files the user has to only copy the configuration file to the
.config file. Example bellow:

    $ cp ./config/config_linux.cfg .config
    $ make          --> press 'x' when the configuration window appears


    OSAL will start the compilation. 
