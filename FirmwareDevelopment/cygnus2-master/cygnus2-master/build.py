#
# Cygnus2 Build Script
#
# This builds all images in the Cygnus2 project
# Written in Python 3.6
#

import subprocess


# build all MCU configurations with Keil uVision tool
subprocess.call(['C:\\Keil_v5\\UV4\\UV4.exe', '-b', 'Source\\MCU\\Application\\MCU.uvprojx', '-z', '-o', 'build.log'], shell=True)
subprocess.call(['C:\\Keil_v5\\UV4\\UV4.exe', '-b', 'Source\\MCU\\Bootloader\\Bootloader.uvprojx', '-z', '-o', 'build.log'], shell=True)


# build all unit test configurations with MS Visual Studio
subprocess.call(['C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Professional\\MSBuild\\15.0\\Bin\\MSBuild.exe', \
                 'Source\\CUnitTest\\CUnit.sln', '/property:Configuration=Debug'], shell=True)

subprocess.call(['C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Professional\\MSBuild\\15.0\\Bin\\MSBuild.exe', \
                 'Source\\CUnitTest\\CUnit.sln', '/property:Configuration=Release'], shell=True)


# build mesh DLL with MS Visual Studio
subprocess.call(['C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Professional\\MSBuild\\15.0\\Bin\\MSBuild.exe', \
                 'Source\\PC\\meshdll\\Mesh.sln', '/property:Configuration=Debug', '/property:Platform=x86'], shell=True)

subprocess.call(['C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Professional\\MSBuild\\15.0\\Bin\\MSBuild.exe', \
                 'Source\\PC\\meshdll\\Mesh.sln', '/property:Configuration=Debug', '/property:Platform=x64'], shell=True)


