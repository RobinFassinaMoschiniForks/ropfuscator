// ==============================================================================
//   BINARY AUTOPSY
//   part of the ROPfuscator project
// ==============================================================================
// This module extracts useful features from a given binary. We call it
// "autopsy" because we have to analyse an already compiled program, dissecting
// it in many sections and extracting as many features as possible.
//
// More in detail, the module is able to extract:
//      - data about ELF sections
//      - symbols from the .dynsym section
//      - microgadgets from executable section
//
// Microgadgets are a subset of what are commonly known as ROP Gadgets, with the
// only difference that we grab only the ones composed by a single instruction
// before the ret, e.g.:
//        mov eax, ebx
//        ret
//
// This module offers also a set of methods to look for a specific microgadget
// among the ones discovered and a lately described "exchange path" analyser.
//

#ifndef BINAUTOPSY_H
#define BINAUTOPSY_H

#include <bfd.h>
#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <string>
#include <vector>

// Symbol - entry of the dynamic symbol table. We use them as references
// to locate the needed gadgets.
struct Symbol {
  // Label - symbol name.
  char *Label;

  // Version - this is mostly useful when dealing with libc, because within it
  // there are lots of symbols with the same label. GNU LIBC uses versioning to
  // ensure compatibility with binaries using old ABI versions.
  char *Version;

  // SymVerDirective - it is just an inline asm directive we need to place to
  // force the static linker to pick the right symbol version during the
  // compilation.
  char *SymVerDirective;

  // Address - offset relative to the analysed binary file. When we'll reference
  // a gadget in memory we'll use this as base address.
  uint64_t Address;

  // Constructor
  Symbol(std::string label, std::string version, uint64_t address);

  // getSymVerDirective - returns a pointer to the SymVerDirective string.
  char *getSymVerDirective();
};

// Section - section data dumped from the ELF header
struct Section {
  // Label - section name
  std::string Label;

  // Address - offset relative to the analysed binary file.
  // Length - Size of the section.
  uint64_t Address, Length;

  // Constructor
  Section(std::string label, uint64_t address, uint64_t length);
};

// Microgadget - represents a single x86 instruction that precedes a RET.
struct Microgadget {
  // Instr - pointer to a capstone-engine data structure that contains details
  // on the overall semantics of the instruction, along with address, opcode,
  // etc.
  cs_insn *Instr;

  // debug
  std::string asmInstr;

  // Constructor
  Microgadget(cs_insn *instr, std::string asmInstr);

  // getAddress - returns the offset relative to the analysed binary file.
  uint64_t getAddress() const;

  // getID - returns the instruction opcode.
  x86_insn getID() const;

  // getOp - returns the i-th instruction operand.
  cs_x86_op getOp(int i) const;

  // getNumOp - returns the total number of operands of the instruction
  uint8_t getNumOp() const;
};

// BinaryAutopsy - dumps all the data needed by ROPfuscator.
// It provides also methods to look for specific gadgets and performs the
// "exchange path" analysis.
// This class has been designed as singleton to simplify the interaction with
// the ROPChain class. Indeed, we don't want to analyse the same file every time
// that a new ROPChain is instanciated.
class BinaryAutopsy {
private:
  // Singleton
  static BinaryAutopsy *instance;
  BinaryAutopsy(std::string path);

public:
  // Symbols - results from dumpDynamicSymbols() are placed here
  std::vector<Symbol> Symbols;

  // Sections - results from dumpSections() are placed here
  std::vector<Section> Sections;

  // Microgadgets - results from dumpGadgets() are placed here
  std::vector<Microgadget> Microgadgets;

  // BinaryPath - path of the binary file that is being analysed
  char *BinaryPath;

  // BfdHandle - an handle to read ELF headers. It is used by dumpSections() and
  // dumpDynamicSymbols()
  bfd *BfdHandle;

  // getInstance - returns an instance of this singleton class
  static BinaryAutopsy *getInstance(std::string path);

  // dumpSections - parses the ELF header using LibBFD to obtain a list of
  // sections that contain executable code, from which the symbol and gadget
  // extraction will take place.
  void dumpSections();

  // dumpDynamicSymbols - extracts symbols from the .dynsym section. It takes
  // into account only function symbols with global scope and used in executable
  // sections.
  void dumpDynamicSymbols();

  // dumpGadgets - extracts every microgadget (i.e., single instructions
  // before a RET) that can be found in executable sections. Each instruction is
  // decoded using capstone-engine.
  void dumpGadgets();

  // getRandomSymbol - returns a random symbol. This is used to reference each
  // gadget in the ROP chain as sum of a random symbol address and the gadget
  // offset from it.
  Symbol *getRandomSymbol();

  // gadgetLookup - set of overloaded methods to look for a specific gadget in
  // the set of the ones that have been previously discovered.
  Microgadget *gadgetLookup(std::string asmInstr);
};

#endif