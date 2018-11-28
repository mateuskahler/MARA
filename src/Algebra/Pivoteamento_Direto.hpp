/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de classe que representa Matriz.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ALGEBRA_PIVOTEAMENTO_DIRETO_INCLUIDO
#define SIMULADOR_ALGEBRA_PIVOTEAMENTO_DIRETO_INCLUIDO sim

#include <Algebra/Matriz.hpp>

namespace Algebra {

namespace Interno{
	
//esta estrutura acrescenta um "proxy" para representar permutações sobre matrizes
// (sem efetivamente deslocar os elementos na memória)
struct MProxy
{
	Matriz mem;
	inds_t X_ind;
	inds_t Y_ind;
	MProxy(Matriz M) : mem(std::move(M))
	{
		ind_t const L = mem.L;
		ind_t const A = mem.A;
		X_ind.reserve(L);
		Y_ind.reserve(A);
		//inicialmente, cada linha e cada coluna mapeiam para si mesmas (índice crescente de 0 a N-1)
		for(ind_t x=0; x<L; x++)
			{X_ind.emplace_back(x);}
		for(ind_t y=0; y<A; y++)
			{Y_ind.emplace_back(y);}
	}
	NUM_T & acesso(ind_t y, ind_t x)
	{
		return mem.acesso(Y_ind[y], X_ind[x]);
	}
	void trocaLinhas(ind_t yA, ind_t yB)
	{
		std::swap(Y_ind[yA], Y_ind[yB]);
	}
	void trocaColunas(ind_t xA, ind_t xB)
	{
		std::swap(X_ind[xA], X_ind[xB]);
	}
};

} // fim namespace Interno

//função que resolve [x] para o sitema representado por [G]*[x]=[u]
/*
 * A função triangulariza a matriz e resolve por retrosubstituição.
 * 
 * A triangulação efetua permutações para selecionar melhor pivô,
 * mas esta operação é transparente e as incógnitas são retornadas na
 * ordem original
 * */
inline std::vector<NUM_T> Resolve(Matriz G, std::vector<NUM_T> u)
{
	using Interno::MProxy;
	//pré-condições mínimas
		if(static_cast<ind_t>(u.size())!= G.A)
		{
			throw std::runtime_error("numero de constantes != numero de equacoes");
		}
		if(G.A!=G.L)
		{
			throw std::runtime_error("matriz nao quadrada");
		}
	//ordem do sistema
		ind_t const N = G.L;
	//proxys de acesso
		MProxy MP(std::move(G));
		MProxy BP(Matriz::Coluna(std::move(u)));
	//lambda de procura de pivo
		auto melhor_pivo = [&](ind_t const k_inicial) -> std::pair<ind_t, ind_t>
		{
			ind_t y_melhor = -1;
			ind_t x_melhor = -1;
			NUM_T pivo  = 0.;
			// ! pivotemaneto completo (pode ser removido um loop interior para pivoteamento parcial)
			for(ind_t y=k_inicial; y<N; y++)
			{
				for(ind_t x=k_inicial; x<N; x++)
				{
					NUM_T a = std::abs(MP.acesso(y,x));
					if(a>pivo)
					{
						y_melhor = y;
						x_melhor = x;
						pivo = a;
					}
				}
			}
			if(y_melhor < 0)
			{
				throw std::runtime_error("sistema singular");
			}
			return std::pair<ind_t, ind_t>(y_melhor, x_melhor);
		};
	//para cada variável, encontra linha usada para eliminação
		for(ind_t k=0; k<(N-1); k++)
		{
			//posisionalmento do pivô
			{
				auto p = melhor_pivo(k);
				ind_t y = p.first;
				ind_t x = p.second;
				//troca linhas e colunas para posicionar pivô em (k,k)
				MP.trocaLinhas(k, y);
				MP.trocaColunas(k, x);
				BP.trocaLinhas(k, y);
			}
			//valor de elemento pivô
				NUM_T const epv = MP.acesso(k,k); 
			//realiza piveoteamento nas linhas posteriores
				for(int lo = k+1; lo<N; lo++)
				{
					NUM_T a = MP.acesso(lo, k);
					NUM_T g = a/epv; //proporção entre pivô na linha de operação e na linha de pivoteamento
					//calcula coeficiente de cada nova coluna
						MP.acesso(lo, k) = 0.; //assume que pivoteamento anula variável
						for(ind_t x = k+1; x<N; x++)
						{
							NUM_T e_r = MP.acesso(k, x);
							MP.acesso(lo, x) -= ( e_r * g );
						}
					//calcula coeficiente no vetor de constantes
						BP.acesso(lo, 0) -= g*BP.acesso(k, 0) ;
				}
		}
	//retro substituições
		std::vector<NUM_T> sol(N);
		//para cada linha k (de última para primeira)
		for(ind_t k=N-1; k>=0; k--)
		{
			//acumulo para cada coluna referente a variáveis já substituidas
			NUM_T acum = 0.;
			for(ind_t co = k+1; co<N; co++)
				{acum += MP.acesso(k, co) * sol[co];}
			//resolve com constante faltante
				sol[k] = (BP.acesso(k, 0) - acum) /  MP.acesso(k, k);
		}
		//"despermuta" incógnitas
		std::vector<NUM_T> R(N);
		for(ind_t i=0; i<N; i++)
		{
			ind_t origem = MP.X_ind[i];
			R[origem]  = sol[i];
		}
	return R;
}


} //fim namespace Algebra

#endif //fim header guard SIMULADOR_ALGEBRA_PIVOTEAMENTO_DIRETO_INCLUIDO