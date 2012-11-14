#ifndef MOD_SIMULATEPOST_H
#define MOD_SIMULATEPOST_H

#include "X86Module.h"


class Mod_SimulatePost : public X86Module
{
	public:
		Mod_SimulatePost();
		virtual ~Mod_SimulatePost();

		void Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const;
};

#endif // MOD_SIMULATEPOST_H
