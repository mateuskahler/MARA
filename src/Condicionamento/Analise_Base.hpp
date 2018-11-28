/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição basilar das análises e objetivos possíveis.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_ANALISE_BASE_INCLUIDA
#define SIMULADOR_CONDICIONAMENTO_ANALISE_BASE_INCLUIDA sim

#include <Condicionamento/Circuito_Esquematico.hpp>
#include <Circuito/Circuito_de_Componentes.hpp>
#include <Estoque/Estoque.hpp>
#include <Tratamento/Extrator.hpp>

namespace Condicionamento {

struct Analise_Base
{
	static constexpr NUM_T erro_relativo_padrao_estabilidade = 5e-4;
	static constexpr NUM_T erro_absoluto_padrao_estabilidade = 5e-15;
//typedefs
	typedef std::map<std::string, ind_t> mapa_nome_ind_t;
	typedef std::map<ind_t, ind_t>       mapa_ind_ind_t;
//mapeamento
	mapa_nome_ind_t m_comp_nome_i;
	mapa_nome_ind_t m_nodo_nome_i;
//memória
	Circuito::Circuito_de_Componentes circuito_componentes;
	Circuito::Circuito_Equivalente_t  circuito_equivalente;
	Tratamento::Objetivos_t objetivos;
//parâmetros de análise
	NUM_T erro_relativo_estabilidade;
	NUM_T erro_absoluto_estabilidade;
/////////////////////////////////////////////
	virtual std::string Tipo_Analise() const = 0;
	virtual Tratamento::Resultados_t resolve(Condicionamento::Circuito_Esquematico const & circuito, Estoque::Estoque_t const & estoque) = 0;
	virtual ~Analise_Base() = default;
	ind_t Adiciona_Objetivo(Tratamento::Objetivo objetivo)
	{
		ind_t i = static_cast<ind_t>(objetivos.size());
		objetivos.emplace_back(std::move(objetivo));
		return i;
	}
	ind_t Adiciona_Objetivo(std::string componente, std::string propriedade)
	{
		return Adiciona_Objetivo(Tratamento::Objetivo(componente, propriedade));
	}
	ind_t Adiciona_Objetivo(std::string nodo)
	{
		return Adiciona_Objetivo(Tratamento::Objetivo(nodo));
	}
	void carrega_esquematico(Condicionamento::Circuito_Esquematico const & esquema, Contexto_Analise contexto_analise, Estoque::Estoque_t const & estoque)
	{
		//número de componentes
			ind_t const NC = esquema.Componentes.size();
		
		//buffers temporários
			//mapa para conexões de controle. Mapeia índice de componente que as contém para vetor de string com seus valores
				std::map<ind_t, std::vector<std::string>> buf_conexoes_controle_m_i_nome; 
		
		//Estágio 1: gera componentes, mapeia nodos por nome, e insere componente no circuito carregado
		//           salva conexões extras como referências de controle
			m_comp_nome_i.clear();
			m_nodo_nome_i.clear();
			m_nodo_nome_i["0"] = 0; // reserva nome "0" para nodo de referência / terra
			ind_t const NX = esquema.Conexoes.size();
			if(NX != NC)
			{
				throw std::runtime_error("O numero de componentes e de conexoes nao eh o mesmo (cada componente requer suas conexoes)");
			}
			for(ind_t k=0; k<NC; k++)
			{
				//carrega componente conforme descrito no esquemático
					Componente_Esquematico const & c = esquema.Componentes[k];
					auto comp_u = estoque.Carrega_Componente(c.tipo, c.modelo, c.parametros);
					Circuito::Componente & comp = *comp_u;
				//verifica quantidade de conexões requeridas e mapeia nome das conexões para índice de nodos
					ind_t N_conexoes_externas = comp.Conexoes_quantidade();
					auto const & conexoes_no_esquema = esquema.Conexoes[k];
					if(static_cast<ind_t>(conexoes_no_esquema.size()) < N_conexoes_externas)
						{	throw std::runtime_error("conexoes insuficientes para componente");	}
				//gera/coleta índices relativos às conexões externas do componente
					inds_t conexoes_i(N_conexoes_externas);
					for(ind_t xe=0; xe<N_conexoes_externas; xe++)
					{
						if(conexoes_no_esquema[xe].empty())
							{ throw std::runtime_error("Conexao (nodo) sem nome"); }
						ind_t ne = associa_nome_repetivel_id(conexoes_no_esquema[xe], m_nodo_nome_i);
						conexoes_i[xe] = ne;
					}
				//conecta ao circuito carregado e mapeia componente
					ind_t comp_i = circuito_componentes.AdicionaComponente(conexoes_i, std::move(comp_u));
					if(esquema.Componentes[k].nome.empty())
						{ throw std::runtime_error("Componente sem nome"); }
					associa_nome_unico_id( esquema.Componentes[k].nome, comp_i, m_comp_nome_i );
			//conexões extras para controles externos? salva no buffer
				if( N_conexoes_externas < (static_cast<ind_t>(conexoes_no_esquema.size())) )
				{
					buf_conexoes_controle_m_i_nome[comp_i] = std::vector<std::string>(
							conexoes_no_esquema.begin() + N_conexoes_externas, conexoes_no_esquema.end() );
				}
			}
		//Estágio 2: monta circuito
			{
				circuito_equivalente = circuito_componentes.GeraCircuitoInicial(contexto_analise);
			}
			
		//Estágio 3: coleta condições iniciais fornecidas no esquemático
			for(auto const & cie: esquema.Condicoes_Iniciais)
			{
				auto procura = m_comp_nome_i.find(cie.nome_comp);
				if(procura == m_comp_nome_i.end())
					{ throw std::runtime_error("Fornecida condicao para componente inexistente"); }
				ind_t comp_i = procura->second;
				circuito_equivalente.estados_fornecidos[comp_i].emplace_back(cie.valor);
			}
		
		//Estágio 4: interpreta conexões de controle fornecidas no esquemático
			{
				auto  fornecido_it = buf_conexoes_controle_m_i_nome.begin();
				ind_t fornecido_s_pos = 0;
				auto requerido_it = circuito_equivalente.controles_requeridos.begin();
				while(requerido_it != circuito_equivalente.controles_requeridos.end())
				{
					ind_t const reqe_i = requerido_it->controles_i[0]; //índice do componente para o qual os controles são requisitados
					//continua percorrendo controles fornecidos
						if(fornecido_it == buf_conexoes_controle_m_i_nome.end())
							{ throw std::runtime_error("Quantidade de controles insuficiente no esquematico"); }
						ind_t const forn_i = fornecido_it->first; //índice do componente para o qual os controles foram fornecidos
						if(forn_i != reqe_i)
							{ throw std::runtime_error("Quantidade de controles erronea no esquematico"); }
						ind_t const fornecido_s_N = fornecido_it->second.size();
						if(requerido_it->corrente)
						{
							//coleta nome de componente de referência
							if(fornecido_s_pos >= fornecido_s_N)
								{ throw std::runtime_error("Quantidade de controles insuficiente no esquematico"); }
							std::string comp_ref_nome = fornecido_it->second[fornecido_s_pos];
							fornecido_s_pos++;
							auto procura_ind_subc_c = circuito_equivalente.m_ind_c_s.find(reqe_i);
							auto procura_ind_subc_r = m_comp_nome_i.find(comp_ref_nome);
							if((procura_ind_subc_c == circuito_equivalente.m_ind_c_s.end()) || (procura_ind_subc_r == m_comp_nome_i.end()))
								{ throw std::runtime_error("Controle fornecido referencia componente inexistente"); }
							ind_t ind_subc_c = procura_ind_subc_c->second;
							ind_t ind_subc_r = circuito_equivalente.m_ind_c_s[procura_ind_subc_r->second];
							circuito_equivalente.circuito.DefineControleCorrente(ind2_t{ind_subc_c, requerido_it->controles_i[1]}, ind2_t{ind_subc_r, 0});
						}
						else
						{
							//coleta nome de nodos referência
							if(fornecido_s_pos >= fornecido_s_N)
								{ throw std::runtime_error("Quantidade de controles insuficiente no esquematico"); }
							std::string nodo_refp_nome = fornecido_it->second[fornecido_s_pos];
							fornecido_s_pos++;
							if(fornecido_s_pos >= fornecido_s_N)
								{ throw std::runtime_error("Quantidade de controles insuficiente no esquematico"); }
							std::string nodo_refn_nome = fornecido_it->second[fornecido_s_pos];
							fornecido_s_pos++;
							auto procura_ind_subc_c = circuito_equivalente.m_ind_c_s.find(reqe_i);
							auto procura_ind_np_r = m_nodo_nome_i.find(nodo_refp_nome);
							auto procura_ind_nn_r = m_nodo_nome_i.find(nodo_refn_nome);
							if((procura_ind_subc_c == circuito_equivalente.m_ind_c_s.end()) || (procura_ind_np_r == m_nodo_nome_i.end()) || (procura_ind_nn_r == m_nodo_nome_i.end()))
								{ throw std::runtime_error("Controle fornecido referencia nodo inexistente"); }
							ind_t ind_subc_c = procura_ind_subc_c->second;
							ind_t ind_np_r = procura_ind_np_r->second;
							ind_t ind_nn_r = procura_ind_nn_r->second;
							circuito_equivalente.circuito.DefineControleDDP(ind2_t{ind_subc_c, requerido_it->controles_i[1]}, ind_np_r, ind_nn_r);
						}
					//avança iteração pelos controles fornecidos e requeridos
					if(fornecido_s_pos == fornecido_s_N)
						{ fornecido_it++; fornecido_s_pos=0; }
					requerido_it++;
				}
				if(requerido_it != circuito_equivalente.controles_requeridos.end())
					{ throw std::runtime_error("Quantidade de controles insuficiente no esquematico"); }
			}
	} //.fim da expansão do esquemático
protected:
	Analise_Base() 
		: erro_relativo_estabilidade(erro_relativo_padrao_estabilidade),
			erro_absoluto_estabilidade(erro_absoluto_padrao_estabilidade){;}
/////////////////////////////////////////////////////////////////////////////
//funções auxiliares internas
/////////////////////////////////////////////////////////////////////////////
	bool relativamente_iguais(NUM_T a, NUM_T b)
	{
		NUM_T dif_a  = std::abs(a-b);
		if(dif_a<=erro_absoluto_estabilidade)
			{ return true; }
		NUM_T a_a = std::abs(a);
		NUM_T b_a = std::abs(b);
		if((a_a<=erro_absoluto_estabilidade) && (b_a<=erro_absoluto_estabilidade))
			{ return true; }
		NUM_T soma_a = a_a + b_a;
		NUM_T rel_dif = NUM_T(2)*dif_a/soma_a;
		return (rel_dif <= erro_relativo_estabilidade);
	}
//função auxiliar que adiciona nome único ao mapa e não pode falhar (gerando erro)
	ind_t associa_nome_unico_id(std::string nome, ind_t id, mapa_nome_ind_t & mapa)
	{
		auto procura = mapa.find(nome);
		if(procura == mapa.end())
		{
			mapa[nome] = id;
			return id;
		}
		//nome já existente: erro
		std::stringstream ss;
		ss << "nome duplicado: " << nome;
		throw std::runtime_error(ss.str());
	}
//função auxiliar que adiciona nome novo ao mapa ou retorna índice pré-associado
	ind_t associa_nome_repetivel_id(std::string nome, mapa_nome_ind_t & mapa)
	{
		auto procura = mapa.find(nome);
		if(procura == mapa.end())
		{
			ind_t novo_id = mapa.size();
			mapa[nome] = novo_id;
			return novo_id;
		}
		else
		{
			return procura->second;
		}
	}
	//a validação dos objetivos deve ocorrer após contrução do circuito equivalente
	//  (pois os componentes e nodos estarão mapeados)
	bool valida_objetivos()
	{
		for(auto const & obj : objetivos)
		{
			if(obj.nodal)
			{
				auto procura_comp = m_nodo_nome_i.find( obj.nome );
				if(procura_comp == m_nodo_nome_i.end())
				{
					std::stringstream ss;
					ss << "objetivo de analise referencia nodo \'" << obj.nome;
					ss << "\', que nao existe no esquematico.";
					throw std::runtime_error(ss.str());
				}
			}
			else
			{
				auto procura_comp = m_comp_nome_i.find( obj.nome );
				if(procura_comp == m_comp_nome_i.end())
				{
					std::stringstream ss;
					ss << "objetivo de analise referencia componente \'" << obj.nome;
					ss << "\', que nao existe no esquematico.";
					throw std::runtime_error(ss.str());
				}
				if(!(circuito_componentes.Componente_Acesso(procura_comp->second).possui_propriedade( obj.propriedade )) )
				{
					std::stringstream ss;
					ss << "objetivo de analise requesita \'" << obj.propriedade;
					ss << "\' de componente " <<  obj.nome << ", ";
					ss << "de tipo \'" << circuito_componentes.Componente_Acesso(procura_comp->second).tipo_str() << "\', ";
					ss << "que nao possui esta propriedade";
					throw std::runtime_error(ss.str());
				}
			}
		}
		return true;
	}
	bool atualiza_modelo_se_instavel(Circuito::Modelo_Ramificado novo_modelo, ind_t subcircuito_ind)
	{
		auto const & a_ramos_i = circuito_equivalente.circuito.RamosiDeSubcircuito(subcircuito_ind);
		auto const & n_ramos_i = novo_modelo.circuito.RamosIndexados();
		if(a_ramos_i.size() != n_ramos_i.size())
			{ throw std::runtime_error("atualiza_modelo_instavel(): modelo requer circuito com diferente numero de ramos para estabilizar"); }
		ind_t r_N = a_ramos_i.size();
		bool ramos_iguais = true;
		for(ind_t r_pos = 0; r_pos<r_N; r_pos++)
		{
			//referência para ramo atual e novo ramo proposto
				auto & ramo_a = circuito_equivalente.circuito.CircuitoAcesso().ramos[a_ramos_i[r_pos]].elemento;
				auto const & ramo_n = novo_modelo.circuito.RamoConectadoLeitura(n_ramos_i[r_pos]).elemento;
			if(ramo_a.tipo != ramo_n.tipo)
				{ throw std::runtime_error("atualiza_modelo_instavel(): modelo requer ramos de diferentes tipos para estabilizar"); }
			//verifica se ramos são equivalentes e atualiza parâmetros se necessário
				NUM_T pr_a = ramo_a.pr;
				NUM_T pr_n = ramo_n.pr;
				if(!relativamente_iguais(pr_a, pr_n))
				{
					ramo_a.pr = pr_n;
					ramos_iguais = false;
				}
		}
		return (!ramos_iguais);
	}
}; //fim da classe Analise_Base

} //fim namespace Condicionamento

#endif //fim header guard SIMULADOR_CONDICIONAMENTO_ANALISE_BASE_INCLUIDA