/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de classe que Integrador Numérico.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ALGEBRA_INTEGRADOR_INCLUIDO
#define SIMULADOR_ALGEBRA_INTEGRADOR_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

namespace Algebra {

struct Integrador
{
	Integrador()        : Y(static_cast<NUM_T>(0)), k(static_cast<NUM_T>(1)){;}
	Integrador(NUM_T k) : Y(static_cast<NUM_T>(0)), k(k){;}
	NUM_T Y;
	NUM_T k;
	NUM_T integra_passo(NUM_T x, NUM_T dt)
	{
		Y += k*x*dt;
		return Y;
	}
};

} //fim namespace Algebra

#endif //fim header guard SIMULADOR_ALGEBRA_INTEGRADOR_INCLUIDO