//modules.h

/*
*	This file contains the key function declarations used in the BIBIFI project.
*	Include this file in the main.
*/

#ifndef MODULES_H
#define MODULES_H

#include <bits/stdc++.h> //this also includes string
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>

namespace filesys = std::filesystem;
using njson = nlohmann::json;


using namespace std; //asume that is std

// global variables - initialized in main.cpp
extern std::string base_anchor_path;
extern std::string base_path;
extern std::string dir_ptr;
extern std::string session_username;
extern njson * system_tree;

// core functions
std::string sys_getcwd();
std::string ConvertArgv(char *);
std::string ValidateKey(bool *, std::string);
std::string SanitizePath(std::string, bool *);
njson sys_load_keymapper();
njson * SystemTraversal(std::string);
njson GenerateKeyfile(std::string);
bool CheckFilesys();
bool DirectoryCheck(njson *);
bool PermissionCheck(njson *, std::string);
bool OwnerCheck(njson *, std::string);
bool VerifyLayer(njson *);
void InitSetup();
void AdminCreateUser(const std::string& username);
bool AddCollaborator(const std::string& filePath, const std::string& targetUser);

// helper functions
void sys_load_system_tree();
void HelperWriteJson(filesys::path, njson);
njson HelperBuildInitSystem();
std::string SubtractSubstring(std::string, std::string);
void WriteChangeToSystemJsonFile();
bool UpdateSharedJsonAddCollab(const std::string& filePath, const std::string& targetUser);
bool UpdateSystemJsonAddCollab(const std::string&, const std::string&);
njson GetSharedObjects(); 

// user functions
std::vector<std::string> tokenize(std::string);
std::string getUserInput();
void getAndExecuteCommand(bool *running);
void pwd(); 
void cd(std::string); 
void ls();
void cat(std::vector<std::string>); 
void cat2(std::vector<std::string>);
void share(std::vector<std::string>); 
void mkfile(std::vector<std::string>); 
void mkdir(std::vector<std::string>); 
void exitmodule(bool * running);
void adduser(std::vector<std::string>);

/*
//cipher functions
std::string Encrypt(std::string inputString, std::string userKey, int * plainLength);
std::string Decrypt(std::string ciphertext, std::string userKey, int * cipherLength);
int aesInit(unsigned char * userKey, unsigned int keyLen, unsigned char * salt, EVP_CIPHER_CTX * ctx, bool encrypt);*/


// other functions

njson ReadKeyfile(std::string);


#endif
