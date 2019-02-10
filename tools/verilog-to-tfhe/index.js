const assert = require("assert");
const fs = require("fs");
const parser = require("./parser.js");

if (process.argv.length != 3) {
	console.error(`Syntax: ${process.argv[0]} ${process.argv[1]} file.v`);
	process.exit(1);
}

const target = process.argv[2];
const source = fs.readFileSync(target, "utf8");
// If we stripped comments ourselves, the line numbering in parser errors would be off.
if (/\(\*[^*]+\*\)\n/gm.test(source)) {
	console.error("The file contains comments, please strip them:\n\n    grep -Fv '(*' original.txt > stripped.txt\n");
	process.exit(2);
}

let ast;
try {
	ast = parser.parse(source);
} catch (e) {
	console.error(`[${target}:${e.location.start.line}:${e.location.start.column}] ${e.message}`);
	process.exit(3);
}
// Only one module at a time is supported
assert(ast.length == 1);
const _ = ast[0];

let buf = "/* Generated automatically by CapacitorSet/FHE-tools */\n\n";

const outputs = _.decls
	.filter(it => it.type === "output")
	.map(it => (it.size == 1 ? "bit_t" : "bitspan_t") + " " + it.name);
const inputs = _.decls
	.filter(it => it.type === "input")
	.map(it => "const " + (it.size == 1 ? "bit_t" : "bitspan_t") + " " + it.name);
let prototype = `void ${_.name}(${outputs.concat(inputs).join(", ")})`;

buf += `${prototype};

${prototype} {
`;

// Maps variables to their type (wire, input or output)
const vars = new Map();
for (const decl of _.decls)
	vars.set(decl.name, decl.type);

const wires = _.decls
	.filter(it => it.type === "wire")
	.map(it => {
		if (it.size === 1)
			return `  bit_t ${it.name} = make_bit();\n`;
		else
			return `  bitspan_t ${it.name} = make_bitspan(${it.size});\n`;
	})
	.join("");
buf += wires;

// Assignments shouldn't appear often in optimized code, and at this time they're not implemented
// correctly (we need to keep track of the width of variables).
assert(_.assignments.length == 0);
/*
// Assignments like "wire <- (input/constant)" are to be put first.
function is_initial_assignment(it) {
	const srcType = it.src.type == "constant" ? "constant" : vars.get(it.src.value);
	const dstType = vars.get(it.dst.value);
	return (srcType == "input" || srcType == "constant") && (dstType == "wire");
};
// Assignments like "output <- (wire/input/constant)" are to be put last.
function is_final_assignment(it) {
	const srcType = it.src.type == "constant" ? "constant" : vars.get(it.src.value);
	const dstType = vars.get(it.dst.value);
	return (srcType == "input" || srcType == "wire" || srcType == "constant") && (dstType == "output");
};
const unexpected_assignments = _.assignments
	.filter(it => !is_initial_assignment(it))
	.filter(it => !is_final_assignment(it));
if (unexpected_assignments.length != 0) {
	console.error("Some assignments are neither initial nor final:");
	console.error(JSON.stringify(unexpected_assignments, null, 2));
	process.exit(4);
}

function transpile_assignment(it) {
	const {src, dst} = it;
	assert(dst.type == "identifier");
	if (!dst.index || !dst.index.end) {
		const cpp_dst = dst.index ? `${dst.value}[${dst.index.begin}]` : dst.value;
		switch (src.type) {
		case "constant":
			assert(!dst.index);
			assert(dst.type == "identifier");
			return `  _constant(${cpp_dst}, 0x${src.hex});\n`;
		case "identifier":
			if (src.index) {
				assert(!src.index.end);
				return `  _copy(${cpp_dst}, ${src.value}[${src.index.begin}]);\n`;
			} else {
				return `  _copy(${cpp_dst}, ${src.value});\n`;
			}
		default:
			assert(false);
		}
	} else {
		assert(false);
	}
}
const initial_assignments = _.assignments
	.filter(is_initial_assignment)
	.map(transpile_assignment)
	.join("");
buf += initial_assignments;
*/

// Add indices where necessary
function transpile_identifier(it) {
	assert(it.type == "identifier");
	if (!it.index)
		return it.value;
	assert(!it.index.end);
	return it.value + "[" + it.index.begin + "]";
}

const gates = _.gates
	.map(it => {
		if (it.gate == "mux")
			return `  _mux(${transpile_identifier(it.z)}, ${transpile_identifier(it.sel)}, ${transpile_identifier(it.t)}, ${transpile_identifier(it.f)});\n`;
		else
			return `  _${it.gate}(${transpile_identifier(it.z)}, ${transpile_identifier(it.a)}, ${transpile_identifier(it.b)});\n`;
	})
	.join("");
buf += gates;

/*
const final_assignments = _.assignments
	.filter(is_final_assignment)
	.map(transpile_assignment)
	.join("");
buf += final_assignments;
*/

buf += "}\n"

console.log(buf);