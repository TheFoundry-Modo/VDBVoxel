# VDBVoxel
VDBVoxel plugin in Modo

## Build

As prerequisites, you have to build [OpenVDB](http://www.openvdb.org/) firstly and get a [MODO](https://www.thefoundry.co.uk/products/modo/) with [MODO SDK](http://modo.sdk.thefoundry.co.uk/wiki/Main_Page) before building this plugin.

### Build with Visual Studio 2013

The master repository provides a Visual Studio 2013 project file, which includes two projects: *common* and *VDBVoxel*. In order to make them work for you, you may need to:

1. Change the including path macros inside the file *\build\Vistual Studio 2013\PropertySheet.props*, which is a XML file that can be loaded by common text editors and Visual Studio. You can specify the root path of the directories of TBB, OPENVDB, MODO SDK and BOOST by editing the corresponding *UserMacros* in the file.

2. Add the **common** source files of MODO SDK to the visual studio project, which is also named *common*.

3. Link OpenVDB lib(s) and any missing lib(s). The plugin itself, only needs to specify OpenVDB lib(s). However, some libs of OpenVDB dependencies may need to be linked as well, if they were not linked when building OpenVDB lib(s).