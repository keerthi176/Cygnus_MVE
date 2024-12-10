from distutils.core import setup, Extension
import os

# Make the extension module ("mymodule") link against xosd
module1 = Extension('PY_MC_PUP',
                    include_dirs=["..\..\..\MCU\Common", "..\..\..\MCU\Mesh", "..\..\..\MCU\MainModule", "..\..\..\MCU\Configuration", "..\..\..\MCU\DeviceManager", "..\..\..\MCU\DeviceManager\STM32L4xx_HAL_Driver", "..\..\..\MCU\DeviceManager\STM32L4xx_HAL_Driver\Inc", "..\..\meshdll", "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.16299.0\\ucrt"],
					library_dirs =["C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.15063.0\\ucrt\\x64"],
                    sources = ['PY_MC_PUP.c'])

setup (name = 'PY_MC_PUP',
       version = '1.0',
       description = 'MC_PUP wrapper package',
       ext_modules = [module1])

