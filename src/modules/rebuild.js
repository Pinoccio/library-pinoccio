var fs = require('fs');

// do the Modules.h
var header = fs.createWriteStream('Modules.h');
header.write('// DO NOT EDIT, auto-generated from rebuild.js\n\n');
header.write('#include "PinoccioModule.h"\n\n');

fs.readdirSync('.').forEach(function(file){
  if(fs.lstatSync(file).isDirectory())
  {
    header.write('#include "'+file+'/'+file+'.h"\n');
  } 
});

header.write('\nPinoccioModule *ModulesNamed(const char *name);\n');
header.write('void ModulesPrint();\n');
header.end();

// do the Modules.cpp
var cpp = fs.createWriteStream('Modules.cpp');
cpp.write('// DO NOT EDIT, auto-generated from rebuild.js\n\n');
cpp.write('#include <Scout.h>\n');
cpp.write('#include "Modules.h"\n\n');
cpp.write('PinoccioModule *ModulesNamed(const char *name)\n{\n');

fs.readdirSync('.').forEach(function(file){
  if(fs.lstatSync(file).isDirectory())
  {
    cpp.write('  if(strcmp("'+file.toLowerCase()+'",name) == 0) return (PinoccioModule*)(new '+file+'Module());\n');
  } 
});

cpp.write('  return NULL;\n}\n\n');
cpp.write('void ModulesPrint()\n{\n');

fs.readdirSync('.').forEach(function(file){
  if(fs.lstatSync(file).isDirectory())
  {
    cpp.write('  speol("'+file.toLowerCase()+'");\n');
  } 
});

cpp.write('}\n');
cpp.end();

