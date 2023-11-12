#!/usr/bin/env python3

import subprocess
import os
import shutil
import argparse

# note you will want to chmod +x if you want to run this file without python3 prefix

source_files = ["main.cpp", "TextProcessing.cpp", "Filesys.cpp", "input.cpp", "ShellCommands.cpp", "UserManagement.cpp", "cipher.cpp"]

executable = "executable"

def remove_executable():
	if(os.path.exists(f"./{executable}")):
		os.remove(f"./{executable}")
		print(f"Removed {executable}.")

	if(os.path.exists(f"./keyfiles/")):
		shutil.rmtree(f"./keyfiles/")
		print(f"Removed ./keyfiles/")

	if(os.path.exists(f"./filesys/")):
		shutil.rmtree(f"./filesys/")
		print(f"Removed ./filesys/")

def build_executable():
	command = f"g++ -std=c++17 -fstack-protector -o {executable} " + " ".join(source_files) + " -lssl -lcrypto"
	print(command)
	try:
		subprocess.run(command, shell=True)
		print(f"Finished building {executable}.")
	except Exception as err:
		print(f"Failed to build.")


# main

parser = argparse.ArgumentParser(description="Build script")
parser.add_argument("decision", choices=["remove", "build"])

args = parser.parse_args()

if("remove" in args.decision):
	remove_executable()

elif("build" in args.decision):
	build_executable()















