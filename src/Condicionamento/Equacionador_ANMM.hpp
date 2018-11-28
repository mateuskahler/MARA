/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Análise Nodal Modificada Modificada
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_ANMM_INCLUIDO
#define SIMULADOR_CONDICIONAMENTO_ANMM_INCLUIDO sim

#include <Algebra/Sistema_Linear.hpp>
#include <Circuito/Circuito_Ramificado.hpp>
#include <Condicionamento/Grafo.hpp>

namespace Condicionamento
{

struct Equacao_compacta_t
{
	std::vector<std::pair<ind_t, NUM_T>> termos;
	NUM_T constante;
	Equacao_compacta_t():constante(0){;}
	Equacao_compacta_t & operator*=(NUM_T const k)
	{
		for(auto &termo:termos)
			{	termo.second *= k; }
		constante *= k;
		return *this;
	}
};

struct Sistema_ANMM
{
	//memória
	Algebra::Sistema_Linear sistema_linear;
protected:
	std::map<ind_t, ind_t> m_nodos_circ_graf;
	inds_t                 m_nodos_graf_circ;
	std::map<ind_t, ind_t> m_ramos_circ_graf;
	inds_t                 m_ramos_graf_circ;
	std::map<ind_t, ind_t> m_fonteVic_iaux;
	ind_t N_nodos;
public:
	Sistema_ANMM() = default;
	Sistema_ANMM(Circuito::Circuito_Ramificado const & circuito_equivalente)
	{
		m_ramos_graf_circ = circuito_equivalente.RamosIndexados();
		m_nodos_graf_circ = circuito_equivalente.NodosIndexados();
		ind_t const M = m_ramos_graf_circ.size();
		ind_t const N = m_nodos_graf_circ.size();
		N_nodos = N;
		// gera grafo e mapeia ramos e nodos do circuito para o sistema
			Topologia::Grafo grafo(N);
			for(ind_t k=0; k<N; k++)
			{
				m_nodos_circ_graf[m_nodos_graf_circ[k]] = k;
			}
			for(ind_t k=0; k<M; k++)
			{
				ind_t const ri_c = m_ramos_graf_circ[k];
				ind_t const np = m_nodos_circ_graf[ circuito_equivalente.RamoConectadoLeitura(ri_c).conexoes[0] ];
				ind_t const nn = m_nodos_circ_graf[ circuito_equivalente.RamoConectadoLeitura(ri_c).conexoes[1] ];
				m_ramos_circ_graf[ri_c] = grafo.AdicionaAresta(np, nn);
			}
		//validação topológica
			{
				auto cA = grafo.NenhumRamoSingular();
				auto cB = grafo.CaminhosFechados();
				auto cC = grafo.TodosNodosConectados();
				if( !((cA.first) && (cB.first) && (cC)) )
				{
					throw std::logic_error("circuito topologicamente invalido");
				}
			}
		// coleta quantidade de equações auxiliares requeridas (para fontes de diferença de potencial)
			for(ind_t k=0; k<M; k++)
			{
				ind_t const ri_c = m_ramos_graf_circ[k];
				switch(circuito_equivalente.RamoConectadoLeitura(ri_c).elemento.tipo)
				{
					case Circuito::Ramo_Tipo_enum_t::DDP_Fixa: //[[fallthrough]];
					case Circuito::Ramo_Tipo_enum_t::DDP_C_DDP: //[[fallthrough]];
					case Circuito::Ramo_Tipo_enum_t::DDP_C_Corrente: 
						{
							ind_t i = m_fonteVic_iaux.size();
							m_fonteVic_iaux[ri_c] = i;
						}
						break;
					case Circuito::Ramo_Tipo_enum_t::Corrente_Fixa: //[[fallthrough]];
					case Circuito::Ramo_Tipo_enum_t::Corrente_C_DDP: //[[fallthrough]];
					case Circuito::Ramo_Tipo_enum_t::Corrente_C_Corrente: //[[fallthrough]];
					case Circuito::Ramo_Tipo_enum_t::Resistivo:
						break;
					default:
						throw std::runtime_error("GeraSistema() : Ramo basico de tipo nao suportado.");
				}
			}
		//calcula ordem e inicializa memória para sistema linear
			ind_t const aux1_N = m_fonteVic_iaux.size();
			ind_t const ordem  = N+aux1_N-1;
			sistema_linear = Algebra::Sistema_Linear(ordem);
			for(ind_t k=0; k<M; k++)
			{
				ind_t const ri_c = m_ramos_graf_circ[k];
				Estampa_Ramo(circuito_equivalente, ri_c);
			}
	} //fim de construtor do Sistema
	//retorna estado de ramo específico após análise
	Circuito::Estado_Ramo Extrai_Estado_de_Ramo(
		NUMs_t const & resultado , Circuito::Circuito_Ramificado const & circuito,
		ind_t const ramo_ic)const
	{
		Circuito::Estado_Ramo estado;
		estado.i = Resolve_Equacao_Sobre_Ramo( Gera_Eq_Corrente(circuito, ramo_ic), resultado);
		estado.u = Resolve_Equacao_Sobre_Ramo( Gera_Eq_DDP(circuito, ramo_ic), resultado );
		return estado;
	}
	//retorna estado de componente específico após análise
	Condicionamento::Estado_Subcircuito Compila_Estado_Componente(
		NUMs_t const & resultado , Circuito::Circuito_Ramificado const & circuito,
		inds_t const & ramos_i, inds_t const & conexoes_i) const
	{
		Condicionamento::Estado_Subcircuito estado;
		//coleta tensões nos nodos de conexão
			for(auto const & c: conexoes_i)
			{
				ind_t nodo_indice = m_nodos_circ_graf.at(c)-1;
				if(nodo_indice>=0)
				{
					estado.potenciais_conexoes.emplace_back( resultado[nodo_indice] );
				}
				else
				{
					estado.potenciais_conexoes.emplace_back( 0 );
				}
			}
		//coleta estados nos ramos
			for(auto const & r: ramos_i)
			{
				estado.e_ramos.emplace_back( Extrai_Estado_de_Ramo(resultado, circuito, m_ramos_circ_graf.at(r)) );
			}
		return estado;
	}
	NUM_T Extrai_Estado_de_Nodo(NUMs_t const & resultado, ind_t const & nodo_i)const
	{
		auto nodo_ig = m_ramos_circ_graf.at(nodo_i)-1;
		if(nodo_ig < 0) //índice negativo indica que foi requisitada tensão no nodo comum/terra
		{
			return static_cast<NUM_T>(0);
		}
		return resultado[nodo_ig];
	}
protected:
	template <bool SOMA>
	void Estampa_Equacao(Equacao_compacta_t const EQ, ind_t linha)
	{
		//linha negativa indica que equação deveria ser estampada relativa ao nodo de referência, que foi eliminado
		if(linha < 0)
			{return;}
		for(auto const & termo:EQ.termos)
		{
			if(SOMA)
				{	sistema_linear.A.acesso(linha, termo.first) += termo.second; }
			else
				{	sistema_linear.A.acesso(linha, termo.first) -= termo.second; }
		}
		if(SOMA)
			{	sistema_linear.y[linha] += EQ.constante; }
		else
			{	sistema_linear.y[linha] -= EQ.constante; }
	}
	Equacao_compacta_t Gera_Eq_Corrente(
		Circuito::Circuito_Ramificado const & circuito,
		ind_t const ramo_ic ) const
	{
		Equacao_compacta_t EQ;
		auto const & ramo_c     = circuito.RamoConectadoLeitura(ramo_ic).elemento;
		auto const & conexoes_c = circuito.RamoConectadoLeitura(ramo_ic).conexoes;
		ind_t const np = m_nodos_circ_graf.at(conexoes_c[0])-1; //nodo positivo (descontado índice do nodo comum)
		ind_t const nn = m_nodos_circ_graf.at(conexoes_c[1])-1; //nodo negativo (descontado índice do nodo comum)
		switch(ramo_c.tipo)
		{
			case Circuito::Ramo_Tipo_enum_t::Resistivo:
				{
					NUM_T R = ramo_c.pr;
					NUM_T S = static_cast<NUM_T>(1)/R;
					if(np>=0)
						{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(np,  S)); }
					if(nn>=0)
						{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(nn, -S)); }
				}
				break;
			//corrente de fontes de ddp são expressas pela incógnita adicionada pela equação auxiliar
			case Circuito::Ramo_Tipo_enum_t::DDP_Fixa:
			case Circuito::Ramo_Tipo_enum_t::DDP_C_Corrente:
			case Circuito::Ramo_Tipo_enum_t::DDP_C_DDP:
				{
					ind_t aux_L = N_nodos + m_fonteVic_iaux.at(ramo_ic) - 1;
					EQ.termos.emplace_back( std::pair<ind_t, NUM_T>(aux_L, static_cast<NUM_T>(1)) );
				}
				break;
			//corrente de fontes de corrente propagam seu equacionamento próprio
			case Circuito::Ramo_Tipo_enum_t::Corrente_Fixa:
				{
					//fontes de corrente fixa têm seu valor introduzido diretamente no resultado líquido do nodo (constante da equação)
					NUM_T I = ramo_c.pr;
					EQ.constante = -I;
				}
				break;
			case Circuito::Ramo_Tipo_enum_t::Corrente_C_Corrente:
				{
					//fontes de corrente controlada por corrente propagam seu equacionamento
					NUM_T alfa = ramo_c.pr;
					ind_t ramo_ref_c = ramo_c.cp; //índice do ramo usado como referência de corrente
					EQ = (Gera_Eq_Corrente(circuito, ramo_ref_c)*=alfa);
				}
				break;
			case Circuito::Ramo_Tipo_enum_t::Corrente_C_DDP:
				{
					//fontes de corrente controlada por ddp propagam seu equacionamento
					NUM_T alfa     = ramo_c.pr; //ganho da fonte de corrente
					ind_t np_ref_i = m_nodos_circ_graf.at(ramo_c.cp)-1; //traduz índice de nodo de referência positiva do circuito para o sistema
					ind_t nn_ref_i = m_nodos_circ_graf.at(ramo_c.cn)-1; //traduz índice de nodo de referência negativa do circuito para o sistema
					if(np_ref_i>=0)
						{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(np_ref_i,  alfa)); }
					if(nn_ref_i>=0)
						{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(nn_ref_i, -alfa)); }
				}
				break;
			default:
				throw std::runtime_error("Gera_Eq_Corrente() : Ramo basico de tipo nao equacionado.");
		}
		return EQ;
	}
	Equacao_compacta_t Gera_Eq_DDP(
		Circuito::Circuito_Ramificado const & circuito,
		ind_t const ramo_ic ) const
	{
		Equacao_compacta_t EQ;
		auto const & conexoes_c = circuito.RamoConectadoLeitura(ramo_ic).conexoes;
		ind_t const np = m_nodos_circ_graf.at(conexoes_c[0])-1; //nodo positivo (descontado índice do nodo comum)
		ind_t const nn = m_nodos_circ_graf.at(conexoes_c[1])-1; //nodo negativo (descontado índice do nodo comum)
		// na análise nodal, a queda de tensão sobre qualquer ramo é implícita pelas tensões nas conexões
		if(np>=0)
			{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(np, static_cast<NUM_T>(+1))); }
		if(nn>=0)
			{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(nn, static_cast<NUM_T>(-1))); }
		return EQ;
	}
	Equacao_compacta_t Gera_Eq_Auxiliar(
		Circuito::Circuito_Ramificado const & circuito,
		ind_t const ramo_ic )
	{
		Equacao_compacta_t EQ;
		auto const & ramo_c     = circuito.RamoConectadoLeitura(ramo_ic).elemento;
		switch(ramo_c.tipo)
		{
			//DDP fixa : constante
			case Circuito::Ramo_Tipo_enum_t::DDP_Fixa:
				{
					NUM_T V = ramo_c.pr;
					EQ.constante = V;
				}
				break;
			//DDP controlada por DDP : ganho vezes diferença entre nodos de referência
			case Circuito::Ramo_Tipo_enum_t::DDP_C_DDP:
				{
					NUM_T ganho    = ramo_c.pr;
					ind_t np_ref_i = m_nodos_circ_graf[ramo_c.cp]-1; //traduz índice de nodo de referência positiva do circuito para o sistema algébrico
					ind_t nn_ref_i = m_nodos_circ_graf[ramo_c.cn]-1; //traduz índice de nodo de referência negativa do circuito para o sistema algébrico
					if(np_ref_i>=0)
						{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(np_ref_i, -ganho)); }
					if(nn_ref_i>=0)
						{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(nn_ref_i, +ganho)); }
				}
				break;
			//DDP controlada por corrente : ganho vezes corrente no ramo de referência
			case Circuito::Ramo_Tipo_enum_t::DDP_C_Corrente:
				{
					NUM_T ganho      = ramo_c.pr;
					ind_t ramo_ref_i = m_ramos_circ_graf[ramo_c.cp]; //índice do ramo usado como referência de corrente
					EQ = ( Gera_Eq_Corrente(circuito, ramo_ref_i)*=(-ganho) ); //equacão característica
				}
				break;
			default:
				throw std::runtime_error("Gera_Eq_Auxiliar() : Requisitada equacao auxiliar de ramo que nao a possui.");
		}
		//para todas as fontes de tensão, a diferença é aplicada nos nodos aos quais se conecta:
			auto const & conexoes_c = circuito.RamoConectadoLeitura(ramo_ic).conexoes;
			ind_t const np = m_nodos_circ_graf[conexoes_c[0]]-1; //nodo positivo (descontado índice do nodo comum)
			ind_t const nn = m_nodos_circ_graf[conexoes_c[1]]-1; //nodo negativo (descontado índice do nodo comum)
			if(np>=0)
				{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(np, static_cast<NUM_T>(+1))); }
			if(nn>=0)
				{ EQ.termos.emplace_back(std::pair<ind_t, NUM_T>(nn, static_cast<NUM_T>(-1))); }
		return EQ;
	}
	void Estampa_Ramo(
		Circuito::Circuito_Ramificado const & circuito,
		ind_t const ramo_ic )
	{
		auto const & conexoes_c = circuito.RamoConectadoLeitura(ramo_ic).conexoes;
		ind_t const np = m_nodos_circ_graf[conexoes_c[0]]-1; //nodo positivo (descontado índice do nodo comum)
		ind_t const nn = m_nodos_circ_graf[conexoes_c[1]]-1; //nodo negativo (descontado índice do nodo comum)
		//estampa contribuições de corrente em matriz principal
		{
			auto const eqc = Gera_Eq_Corrente(circuito, ramo_ic);
			Estampa_Equacao<false>(eqc, np); //subtrai corrente em nodo positivo
			Estampa_Equacao<true>(eqc, nn);  //adiciona corrente em nodo negativo
		}
		//estampa contribuições de tensão em matriz auxiliar
		// (apenas para fontes de diferença de potencial)
			auto procura_eq_auxiliar = m_fonteVic_iaux.find(ramo_ic);
			if(procura_eq_auxiliar !=  m_fonteVic_iaux.end())
			{
				ind_t aux_L = N_nodos + m_fonteVic_iaux[ramo_ic] - 1; //linha da equação auxiliar requerida
				auto const equ = Gera_Eq_Auxiliar(circuito, ramo_ic);
				Estampa_Equacao<true>(equ, aux_L);  //adiciona corrente em nodo negativo
			}
	} //fim de função para estampar ramo
	NUM_T Resolve_Equacao_Sobre_Ramo(Equacao_compacta_t const & EQ, NUMs_t const & resultado) const
	{
		NUM_T y = -EQ.constante;
		//percorre todos termos da equação, adicionando ao resultado a ponderação dos coeficientes 
		for(auto const & termo:EQ.termos)
		{
			y += resultado[termo.first]*termo.second;
		}
		return y;
	}
public:
	//destrutor
	~Sistema_ANMM() = default;
}; //fim de classe

} //fim namespace Condicionamento

#endif // fim header guard SIMULADOR_CONDICIONAMENTO_ANMM_INCLUIDO