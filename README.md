### Logic Truth Table Generator
***
A C program that reads a digital circuit specification and automatically generates the complete truth table for the circuit.  
The program supports standard logic gates (AND, OR, NOT, XOR, NAND, NOR), PASS-through connections, as well as multi-input components such as DECODER and MULTIPLEXER gates.  

It evaluates circuits with named input/output wires and internal temporary signals, simulating the circuit for every possible input combination.

---

### Features
***
- Reads a structured circuit description file  
- Supports:
  - AND, OR, XOR  
  - NOT  
  - NAND, NOR  
  - PASS  
  - DECODER (n → 2ⁿ)  
  - MULTIPLEXER (2ⁿ → 1)  
- Variables may be constants (`0`, `1`), input wires, outputs, or internal temporary wires.  
- Handles temporary internal wires and discarded outputs (`_`)  
- Automatically determines gate evaluation order, even when internal dependencies are not sorted  
- Produces a full truth table, one row per input combination  
- Efficient row-by-row evaluation without storing the entire table in memory

---

### Input Format
***
Circuit files follow a token-based format:

```
INPUT n  <var1> <var2> ... <varn>
OUTPUT m <out1> <out2> ... <outm>

<GATE> <params...>
```

### Example
```
INPUT 2 A B
OUTPUT 2 C S
AND A B C
XOR A B S
```

### Meaning
- **Inputs:** `A`, `B`  
- **Outputs:**  
  - `C` — carry bit  
  - `S` — sum bit  
- **Gate logic:**  
  - `C = A AND B`  
  - `S = A XOR B`  

This configuration is known as a *half adder*. It computes the binary sum of two input bits.

### Resulting Truth Table
```
A B | C S
0 0 | 0 0
0 1 | 0 1
1 0 | 0 1
1 1 | 1 0
```

Each line shows one possible combination of input bits and the corresponding output produced by the circuit.
The program **does not print the header row** (A B | C S).  

---

### Usage
***
Compile using the provided Makefile:

```
make
```

Run the program with a circuit file:

```
./truthtable data/test.1.01.txt
```

#### Verify Output:
Each test circuit in the `data/` folder has a matching reference file (`ref.*.txt`) containing the correct truth-table output for that circuit.

Compare your program’s output to the corresponding reference file:

```
data/ref.1.01.txt
```
---

### File Structure
***
```
/
├── data/              # Sample circuit specification files
├── truthtable.c       # Full circuit parser and simulator
├── Makefile
└── README.md
```

---
