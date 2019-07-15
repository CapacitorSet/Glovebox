import toposort = require("toposort");
import {transpile_identifier, inputsOf} from "./utils";

type Edge = [string, string];

function gateToEdges(gate: Gate): Edge[] {
	function makeEdge(it: Identifier): Edge {
		return [transpile_identifier(it), transpile_identifier(gate.z)];
	}
	return inputsOf(gate).map(makeEdge);
}

// Returns the list of wires/outputs in topological order.
export default function toposort_gates(gates: Gate[]): string[] {
	const graph: Edge[] = gates
		.map(gateToEdges)
		.reduce((arr, it) => arr.concat(it), []);

	return toposort(graph);
}