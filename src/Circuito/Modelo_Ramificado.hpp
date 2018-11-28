 /**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição de interface de modelo genérico de componente
	 * Cada Modelo Ramificado contém o Circuito Ramificado equivalente e regiões de validade.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CIRCUITO_MODELO_RAMIFICADO_INCLUIDO
#define SIMULADOR_CIRCUITO_MODELO_RAMIFICADO_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>
#include <Auxiliar/Regiao_Operacao.hpp>
#include <Circuito/Circuito_Ramificado.hpp>

namespace Circuito{

//struct referencia_de_controle_t;
//struct referencia_de_estado_inicial_t;

struct Modelo_Ramificado
{
	Circuito_Ramificado circuito;
	//std::vector<referencia_de_controle_t> controles_internos;
	//std::vector<referencia_de_estado_inicial_t> estados_internos;
};


//estrutura que guarda informação sobre a referência de controle para ramos controlados
struct referencia_de_controle_t{
	bool corrente; //se controle é dependente de corrente (se falso, indica que dependente de tensão)
	ind2_t controles_i; //índices da referência
}; //fim  referencia_de_controle_t


} //fim namespace Circuito


#endif //fim header guard SIMULADOR_CIRCUITO_MODELO_RAMIFICADO_INCLUIDO