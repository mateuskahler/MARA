/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de classe que representa Matriz.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ALGEBRA_MATRIZ_INCLUIDA
#define SIMULADOR_ALGEBRA_MATRIZ_INCLUIDA sim

#include <Auxiliar/Dogmas.hpp>

namespace Algebra {

struct Matriz
{
	ind_t A;
	ind_t L;
	NUMs_t mem;
	Matriz(ind_t A, ind_t L) : A(A), L(L), mem(A*L){;}
	//acesso a elemento
		NUM_T& acesso(ind_t y, ind_t x)
		{
			return mem[y*L+x];
		}
	//leitura de elemento
		NUM_T leitura(ind_t y, ind_t x)const
		{
			return mem[y*L+x];
		}
	//gera matriz coluna através de vetor
		static Matriz Coluna(NUMs_t const& v)
		{
			ind_t A = v.size();
			Matriz R(A,1);
			for(ind_t y=0; y<A; y++)
			{
				R.acesso(y,0) = v[y];
			}
			return R;
		}
	~Matriz() = default;
};

} //fim namespace Algebra

#endif //fim header guard SIMULADOR_ALGEBRA_MATRIZ_INCLUIDA