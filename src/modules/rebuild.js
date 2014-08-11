var fs = require('fs');

var dirs = [];
fs.readdirSync('.').forEach(function(file){
  if(fs.lstatSync(file).isDirectory()) dirs.push(file);
});
dirs = dirs.sort();

// do the Modules.h
var header = fs.createWriteStream('Modules.h');
header.write('// DO NOT EDIT, auto-generated from rebuild.js\n\n');
header.write('#include "PinoccioModule.h"\n\n');

dirs.forEach(function(mod){
  header.write('#include "'+mod+'/'+mod+'.h"\n');
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

dirs.forEach(function(mod){
  cpp.write('  if(strcmp("'+mod.toLowerCase()+'",name) == 0) return (PinoccioModule*)(new '+mod+'Module());\n');
});

cpp.write('  return NULL;\n}\n\n');
cpp.write('void ModulesPrint()\n{\n');

dirs.forEach(function(mod){
  cpp.write('  speol("'+mod.toLowerCase()+'");\n');
});

cpp.write('}\n');
cpp.end();

