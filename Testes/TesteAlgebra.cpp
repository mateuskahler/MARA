/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo gera matriz de ordem arbitrária para testar o algorítmo de pivoteamento direto
	 * 
	Mateus Kahler
*/

#include <Algebra/Pivoteamento_Direto.hpp>
#include <Condicionamento/Circuito_Esquematico.hpp>
#include <Tratamento/Extrator.hpp>
#include <random>

int main()
{
	using namespace std;
	ind_t const N = 100;
	vector<NUM_T> xs;
	//gera vetor de números naturais, sem o zero, como resposta do sistema
	// (para facilitar verificação)
		for(ind_t k=0; k<N; k++)
		{
			xs.emplace_back(k+1);
		}
	// gera matriz de valores com expoente e mantissa aleatórios
	// como os valores são todos sorteados, existe uma chance de ser gerado um sistema singular, caso no qual o teste não fornece resultado válido
	//   A troca da semente do gerador aleatório é provavelmente suficiente para condicionar o sistema, dada a baixa possibilidade de ocorrência de matrizes singulares
	// (esta situação nunca foi observada durante os testes)
		Algebra::Matriz M(N,N);
		std::uniform_real_distribution<NUM_T> ae_dist(-12, 12);
		std::uniform_real_distribution<NUM_T> am_dist(-1, 1);
		std::mt19937_64 RNG(42);
		for(ind_t e=0; e<N; e++)
		{
			for(ind_t k=0; k<N; k++)
			{
				M.acesso(e, k) = am_dist(RNG)*std::pow(10., ae_dist(RNG));
			}
		}
	//gera vetor de constantes que representa resposta desejada
		NUMs_t u;
		for(ind_t e=0; e<N; e++)
		{
			u.emplace_back(0.);
			for(ind_t k=0; k<N; k++)
			{
				u.back() += xs[k]*M.acesso(e,k);
			}
		}
	//resolve o sistema para testas método implmentado
		auto x = Algebra::Resolve(M, u);
	//imprime resultado
		std::cout << "Matriz gerada: " << std::endl;
		std::cout << M << std::endl;
		std::cout << "Vetor de constantes: " << std::endl;
		std::cout << u << std::endl;
		std::cout << "Resultado: " << std::endl;
		std::cout << x << std::endl;
}