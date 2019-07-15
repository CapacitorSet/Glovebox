import {parametersOf} from "./utils";

type DeclLifetime = {
	firstDeclared: number;
	lastDeclared: number;
}
type Lifetimes = {[key: string]: DeclLifetime}

export default class LifetimeManager {
	lifetimes: Lifetimes = {};

	constructor(decls: Declaration[]) {
		for (const decl of decls) {
			this.lifetimes[decl.name] = {
				firstDeclared: undefined,
				lastDeclared: undefined
			};
		}	
	}

	populate(_gates: Gate[]) {
		const gates = Array.from(_gates);
		gates.forEach((gate, pos) => {
			for (const ident of parametersOf(gate)) {
				if (this.lifetimes[ident.value].firstDeclared === undefined)
					this.lifetimes[ident.value].firstDeclared = pos;
			}
		});
		const len = gates.length - 1;
		gates.reverse().forEach((gate, i) => {
			const pos = len - i;
			for (const ident of parametersOf(gate)) {
				if (this.lifetimes[ident.value].lastDeclared === undefined)
					this.lifetimes[ident.value].lastDeclared = pos;
			}
		});
	}
}