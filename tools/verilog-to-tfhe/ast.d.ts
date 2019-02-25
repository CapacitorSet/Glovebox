type Wire = {
	type: "wire";
	size: number;
	name: string;
}

type Input = {
	type: "input";
	size: number;
	name: string;
}

type Output = {
	type: "output";
	size: number;
	name: string;
}

type Declaration = Wire | Input | Output

type ArrayIndex = {
	begin: number;
	end: number | null;
}

type Identifier = {
	type: "identifier";
	value: string;
	index?: ArrayIndex;
}

type Gate1 = {
	type: "gate";
	gate: "copy" | "not";
	a: Identifier;
	z: Identifier;
}

type Gate2 = {
	type: "gate";
	gate: "and" | "andyn" | "nand" | "or" | "oryn" | "nor" | "xor" | "xnor";
	a: Identifier;
	b: Identifier;
	z: Identifier;
}

type Mux = {
	type: "gate";
	gate: "mux";
	t: Identifier;
	f: Identifier;
	sel: Identifier;
	z: Identifier;
}

type Gate = Gate1 | Gate2 | Mux;

type Module = {
	type: "module";
	name: string;
	decls: Declaration[];
	gates: Gate[];
}