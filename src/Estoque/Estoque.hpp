/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo define o estoque de todos os componentes disponíveis para simulação
	 * A funcionalidade visa permitir criação de componentes através de uma string representativa de seu tipo.
	 * Como criar um resistor no esquemático através da palavra "Resistor"
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ESTOQUE_ESTOQUE_INCLUSO
#define SIMULADOR_ESTOQUE_ESTOQUE_INCLUSO sim

#include <Estoque/Resistor.hpp>
#include <Estoque/FonteVx.hpp>
#include <Estoque/FonteIx.hpp>
#include <Estoque/Capacitor.hpp>
#include <Estoque/Indutor.hpp>
#include <Estoque/FonteIControladaI.hpp>
#include <Estoque/FonteIControladaV.hpp>
#include <Estoque/FonteVControladaI.hpp>
#include <Estoque/FonteVControladaV.hpp>
#include <Estoque/Diodo.hpp>

namespace Estoque
{
	struct Estoque_t
	{
		Estoque_t(){;}
		typedef std::function<std::unique_ptr<Circuito::Componente>(std::string, NUMs_t)> construtor_componente_t;
		std::map<std::string, construtor_componente_t> lista_construtores;
		std::unique_ptr<Circuito::Componente> Carrega_Componente(std::string tipo, std::string modelo, NUMs_t parametros)const
		{
			auto procura = lista_construtores.find(tipo);
			if(procura != lista_construtores.end() )
			{
				return procura->second(modelo, parametros); 
			}
			else
			{
				std::stringstream ss;
				ss << "Requisitado componente do tipo " << tipo << ", nao carregado em estoque.";
				throw std::runtime_error(ss.str());
				return std::unique_ptr<Circuito::Componente>(nullptr);
			}
		}
		bool verifica_se_possui_componente(std::string tipo)const
		{
			return (lista_construtores.find(tipo)!=lista_construtores.end());
		}
		template <class... TIPOS_DE_COMPONENTES>
		std::vector<std::string> gera_lista_de_componentes()
		{
			lista_construtores.clear();
			adiciona_em_lista<sizeof...(TIPOS_DE_COMPONENTES), TIPOS_DE_COMPONENTES...>();
			return lista_de_componentes();
		}
		std::vector<std::string> lista_de_componentes()
		{
			std::vector<std::string> lista;
			for(auto const & c : lista_construtores)
			{
				lista.emplace_back(c.first);
			}
			return lista;
		}
		static Estoque_t const & Estoque_Pre_Compilado()
		{
			static Estoque_t estoque;
			static bool inicializado = false;
			if(!inicializado)
			{
				using namespace Circuito::Componentes;
				estoque.gera_lista_de_componentes<Capacitor, Resistor, Indutor, FonteVx, FonteIx>();
				inicializado = true;
			}
			return estoque;
		}
	protected:
	//As seguintes funções auxiliares percorrem 'variadic template' contendo todos os tipos de componentes fornecidos
		template <int TIPOS_N, class... TIPOS_DE_COMPONENTES>
			typename std::enable_if_t<(TIPOS_N>=1)> adiciona_em_lista()
			{
				adiciona_em_lista_um_e_ponte<TIPOS_DE_COMPONENTES...>();
			}
		template <int TIPOS_N, class... TIPOS_DE_COMPONENTES >
			typename std::enable_if_t<(TIPOS_N==0)> adiciona_em_lista() { return; }
		template <class TIPO_UM, class... TIPOS_OUTROS>
			void adiciona_em_lista_um_e_ponte()
			{
				extrai_etiqueta_e_construtor<TIPO_UM>();
				adiciona_em_lista<sizeof...(TIPOS_OUTROS), TIPOS_OUTROS...>();
			}
		template<typename COMP_T>
			typename std::enable_if_t<std::is_base_of<Circuito::Componente, COMP_T>::value> extrai_etiqueta_e_construtor()
			{
				decltype(lista_construtores)::key_type chave = COMP_T::Tipo_str();
				typedef std::unique_ptr<Circuito::Componente> comp_unico_t;
				decltype(lista_construtores)::mapped_type construtor = [](std::string modelo, NUMs_t parametros) -> comp_unico_t
					{ return comp_unico_t(new COMP_T(modelo, parametros)); };
				lista_construtores[chave] = construtor;
			}
	}; //fim de classe de Estoque
} //fim de namespace Estoque


#endif //fim header guard SIMULADOR_ESTOQUE_ESTOQUE_INCLUSO