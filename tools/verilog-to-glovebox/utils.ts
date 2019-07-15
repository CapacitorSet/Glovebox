import {strict as assert} from "assert";

// Add indices where necessary
function transpile_identifier(it: Identifier): string {
	if (!it.index)
		return it.value;
	assert(!it.index.end);
	return it.value + "[" + it.index.begin + "]";
}

function inputsOf(gate: Gate): Identifier[] {
	switch (gate.name) {
		case "mux":
			return [gate.sel, gate.t, gate.f];
		case "copy":
		case "not":
			return [gate.a];
		default:
			return [gate.a, gate.b];
	}
}

// Return the output and the inputs of a gate
function parametersOf(gate: Gate): Identifier[] {
	const bits = inputsOf(gate);
	bits.unshift(gate.z);
	return bits;
}

export {transpile_identifier, inputsOf, parametersOf};