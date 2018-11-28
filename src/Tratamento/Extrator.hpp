
#ifndef SIMULADOR_TRATAMENTO_EXTRATOR_INCLUIDO
#define SIMULADOR_TRATAMENTO_EXTRATOR_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>
#include <Algebra/Matriz.hpp>

namespace Tratamento
{
	
	static constexpr bool USA_NOTACAO_CIENTIFICA = false;

	struct Objetivo
	{
		std::string nome;        //nome da origem do objetivo
		std::string propriedade; //propriedade desejada do objetivo
		bool nodal;
		//Objetivos construidos por duas strings assumem que refere-se a nome de componente e propriedade
		Objetivo(std::string componente, std::string propriedade)
			: nome(componente), propriedade(propriedade), nodal(false){;}
		//Objetivos construidos por uma string assumem que refere-se a nome de nodo
		explicit Objetivo(std::string nodo)
			: nome(nodo), propriedade(), nodal(true){;}
		~Objetivo()=default;
	};

	struct Resultado
	{
		Objetivo origem;
		NUMs_t    valor;
		Resultado(Objetivo Origem) : origem(Origem) {;}
	};

	typedef std::vector<Objetivo>  Objetivos_t;
	struct Resultados_t
	{
		std::vector<Resultado> resultados;
		NUMs_t variavel_independente;
	};
	
	
}//fim namespace Tratamento


//métodos para imprimir vetores e matrizes para streams
std::ostream &operator << (std::ostream &os, const inds_t &v)
{
	os << "< ";
	ind_t const N = v.size();
	for(ind_t k=0; k<N; k++)
	{
		os << v[k];
		if(k<(N-1))
			{ os << ", ";}
	}
	os << " >";
	return os;
}

std::ostream &operator << (std::ostream &os, const NUMs_t &v)
{
	auto proc_v = [&](NUM_T x) -> void
		{
			if(Tratamento::USA_NOTACAO_CIENTIFICA)
			{
				os << std::setprecision(6) << std::setw(15) << std::scientific << x;
			}
			else
			{
				os << std::setprecision(4) << std::setw(13) << std::fixed << x;
			}
		};
	os << "< ";
	ind_t const N = v.size();
	for(ind_t k=0; k<N; k++)
	{
		proc_v(v[k]);
		if(k<(N-1))
			{ os << ", ";}
	}
	os << " >";
	return os;
}

std::ostream &operator << (std::ostream &os, const Algebra::Matriz &M)
{
	os << "{ ";
	ind_t const A = M.A;
	ind_t const L = M.L;
	for(ind_t k=0; k<A; k++)
	{
		if(k!=0)
			{ os << "  ";}
		NUMs_t linha(M.mem.begin()+k*L, M.mem.begin()+(k+1)*L);
		os << linha;
		if(k<(A-1))
			{ os << "\n";}
		else
			{ os << " }";}
	}
	return os;
}

//métodos para imprimir Objetivos e Resultados para stream
std::ostream &operator << (std::ostream &os, const Tratamento::Objetivo &obj)
{
	os << std::setw(6) << obj.nome << " , ";
	if(obj.nodal)
	{
		os << std::setw(9) << "(nodo)";
	}
	else
	{
		os << std::setw(9) << obj.propriedade;
	}
	return os;
}

std::ostream &operator << (std::ostream &os, const Tratamento::Resultado &r)
{
	os << r.origem << " = ";
	os << r.valor;
	return os;
}

std::ostream &operator << (std::ostream &os, const Tratamento::Resultados_t &rs)
{
	for(auto const & r:rs.resultados)
	{
		os << r << std::endl;
	}
	return os;
}

std::ostream &operator << (std::ostream &os, const Condicionamento::Circuito_Esquematico &ce)
{
	std::cout << "Circuito \"" << ce.Nome << "\"" << std::endl;
	std::set<std::string> nodos_declarados;
	for(auto const & cnxs:ce.Conexoes)
	{ 
		for(auto const & cx:cnxs)
		{
			nodos_declarados.insert(cx);
		}
	}
	std::cout << "Nodos declarados:" << std::endl << "\t";
	for(auto const & c : nodos_declarados)
	{
		std::cout << c << " ";
	}
	std::cout << std::endl << "Componentes declarados:" << std::endl << "\t";	
	for(auto const & c : ce.Componentes)
	{
		std::cout << c.nome << " ";
	}
	return os;
}

#endif // fim header guard SIMULADOR_TRATAMENTO_EXTRATOR_INCLUIDO