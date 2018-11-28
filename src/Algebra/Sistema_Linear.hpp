/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de classe que representa Sistema Linear.
	 * Um sistema linear matricial na forma
	 *    A * x = y
	 * é representado por uma matriz quadrada [A] e um vetor de constantes [y]
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ALGEBRA_SISTEMA_LINEAR_INCLUIDO
#define SIMULADOR_ALGEBRA_SISTEMA_LINEAR_INCLUIDO sim

#include <Algebra/Matriz.hpp>

namespace Algebra {
	
struct Sistema_Linear
{
	// Sistema do tipo A * x = y
	Matriz A;
	NUMs_t y;
	Sistema_Linear(ind_t const ordem)
		: A(ordem, ordem), y(ordem, static_cast<NUM_T>(0)){;};
	Sistema_Linear():A(0, 0){;};
	~Sistema_Linear()=default;
};
	
} //fim namespace Algebra

#endif //fim header guard SIMULADOR_ALGEBRA_SISTEMA_LINEAR_INCLUIDO