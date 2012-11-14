#ifndef MOD_SIMULATEPRE_H
#define MOD_SIMULATEPRE_H

#include "X86Module.h"


class Mod_SimulatePre : public X86Module
{
	public:
		Mod_SimulatePre();
		virtual ~Mod_SimulatePre();

		void Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const;
};

#endif // MOD_SIMULATEPRE_H
