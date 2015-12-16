/**
 *  \mainpage Operating System Abstraction Layer (OSAL-ESA/ESTEC)
 *
 *  \section intro_sec Introduction
 *
 *  \subsection scope_ssec Scope of the Document
 *
 *  This document defiens the Operating System Abstraction Layer (OSAL)
 *  Application Program Interface (API).
 *
 *  \subsection purpose_ssec Purpose of the document
 *
 *  The aim of the document is to produce the Interface Control Document for the
 *  Operating System Abstraction Layer software building block.
 *
 *  \section osal_sec OSAL
 *
 *
 *  \image html fpss.JPG "Figure 1: OSAL Overview" width=5
 *	
 *  Figure 1 depicts ...
 *  
 *  \subsection osalnit_ssec OSAL Initialisation and Configuration
 *  
 *  
 *  \section install_sec Installation
 *
 *  <b>Step 1:</b> Uncompress the sources
 *
 *  \c$ tar xzvf osal-X.XX.tar.gz
 *
 *  <b>Step 2:</b> Configure and Compile the Sources
 *
 *  \c$ cd osal-X.XX
 *  \c$ make menuconfig
 *
 *  (Configure the library -- press 'x' when done)
 *
 *  \c$ make clean all
 *
 *  <b>Step 3:</b> Install the Library
 *  
 *  \c$ make install
 *
 *  The Library will be installed in the directory selected during
 *  configuration.
 *
 *  Additionally it is possible to select pre-defined configurations. The
 *  directory ./config contains several files with pre-defined configurations.
 *  To use one of these files just:
 *
 *  \c$ cp ./config/<file/you/fancy> .config
 *  \c$ make clean all install
 *
 */

