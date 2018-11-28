 /**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição da interface que todos os componentes devem obedecer,
	 * contendo assinatura das funções que fornecem modelagem requerida para simulações.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CIRCUITO_COMPONENTE_BASE_INCLUIDO
#define SIMULADOR_CIRCUITO_COMPONENTE_BASE_INCLUIDO sim

#include <Circuito/Modelo_Ramificado.hpp>
#include <Auxiliar/Contexto_Analise.hpp>

namespace Circuito{

struct Componente
{
//modelagem
	virtual bool modelo_ajustavel()const = 0;
	virtual Modelo_Ramificado modelo_inicial(Condicionamento::Contexto_Analise const & contexto) = 0;
	virtual Modelo_Ramificado modelo_condicional(Condicionamento::Estado_Subcircuito const & estado,  Condicionamento::Contexto_Analise const & contexto) = 0;
	virtual ind_t Conexoes_quantidade() const = 0;
//extração de propriedades/objetivos de simulação
	virtual bool  possui_propriedade(std::string propriedade) = 0;
	virtual NUM_T extrai_propriedade(std::string propriedade, Condicionamento::Estado_Subcircuito const & estado) = 0;
//informação do componente
	virtual std::string tipo_str()const = 0;
//destrutor
	virtual ~Componente() = default;
};

} //fim namespace Circuito


#endif //fim header guard SIMULADOR_CIRCUITO_COMPONENTE_BASE_INCLUIDO