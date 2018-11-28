/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definições de classe que representa qualquer objeto conectado a um conjunto de nodos.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_AUXILIAR_JOGRAL_INCLUIDO
#define SIMULADOR_AUXILIAR_JOGRAL_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

namespace Auxiliar
{

struct Jogral
{
	explicit Jogral(bool inicia_subindo = true)
	{
		taxa_atual = inicia_subindo?taxa_aceleracao_proposta:NUM_T(1)/taxa_aceleracao_proposta;
		aceleracao = inicia_subindo?1:-1;
		valor_it = 1.5;
		contagem_estavel_fora = 0;
	}
	NUM_T valor_atual()const
	{
		NUM_T valor = valor_it - static_cast<NUM_T>(1);
		return std::max(static_cast<NUM_T>(0), std::min(static_cast<NUM_T>(1), valor));
	}
	ind_t decidido_consecutivos()const
	{
		return contagem_estavel_fora;
	}
	void passo_sobe()
	{
		if(valor_it >= NUM_T(2))
		{
			if(contagem_estavel_fora < contagem_maxima_estabilidade)
				{ contagem_estavel_fora++; }
			return;
		}
		contagem_estavel_fora = 0;
		if(aceleracao == 2)
		{
			taxa_atual *= taxa_aceleracao_proposta;
		}
		else if(aceleracao > 0)
		{
			aceleracao = 2;
		}else
		{
			aceleracao = 1;
			taxa_atual = gera_taxa_complementar(taxa_atual);
		}
		valor_it *= taxa_atual;
	}
	void passo_desce()
	{
		if(valor_it <= NUM_T(1))
		{
			if(contagem_estavel_fora < contagem_maxima_estabilidade)
				{ contagem_estavel_fora++; }
			return;
		}
		contagem_estavel_fora = 0;
		if(aceleracao == -2)
		{
			taxa_atual /= taxa_aceleracao_proposta;
		}
		else if(aceleracao < 0)
		{
			aceleracao = -2;
		}else
		{
			aceleracao = -1;
			taxa_atual = gera_taxa_complementar(taxa_atual);
		}
		valor_it *= taxa_atual;
	}
protected:
	static constexpr NUM_T taxa_aceleracao_proposta = 1.25992; //aproximadamente 2^1/3
	static constexpr ind_t contagem_maxima_estabilidade = 1000;
	static NUM_T gera_taxa_complementar(NUM_T taxa)
	{
		static constexpr NUM_T taxa_aceleracao_minima_superior = 1.0 + 1e-6;
		static constexpr NUM_T taxa_aceleracao_minima_inferior = 1.0 - 1e-6;
		NUM_T nova_taxa = ( (NUM_T(1)/taxa) + (NUM_T(1)/std::sqrt(taxa)) )/NUM_T(2);
		
		if(nova_taxa > NUM_T(1))
			{ nova_taxa = std::max(taxa_aceleracao_minima_superior, nova_taxa);}
		else
			{ nova_taxa = std::min(taxa_aceleracao_minima_inferior, nova_taxa);}
		return nova_taxa;
	}
	NUM_T taxa_atual;
	ind_t aceleracao;
	NUM_T valor_it;
	ind_t contagem_estavel_fora;
};

}//fim de namespace Auxiliar
#endif //fim header guard SIMULADOR_AUXILIAR_JOGRAL_INCLUIDO