#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <queue>

using namespace std;

const string CONVERSION_TYPE_MEALY_TO_MOORE = "mealy-to-moore";
const string CONVERSION_TYPE_MOORE_TO_MEALY = "moore-to-mealy";
const char DELIMETER = ';';
const char SLASH = '/';
const string CHAR_STATE_FOR_MOORE = "q";

struct MealyTransition
{
	string state;
	string out;

	bool operator==(const MealyTransition& other) const
	{
		return state == other.state && out == other.out;
	}
};

namespace std
{
	template <>
	struct hash<MealyTransition>
	{
		size_t operator()(const MealyTransition& transition) const
		{
			// Создаем хеш, комбинируя хеши строк state и out
			return hash<string>()(transition.state) ^ (hash<string>()(transition.out) << 1);
		}
	};
}
struct MealyStateWithTransitions
{
	string currentState;
	vector<MealyTransition> transitions;
};

struct Mealy
{
	vector<string> entries;
	vector<MealyStateWithTransitions> statesWithTransitions;
};

struct MooreStateWithTransitions
{
	string currentState;
	string out;
	vector<string> transitions;
};

struct Moore
{
	vector<string> entries;
	vector<MooreStateWithTransitions> statesWithTransitions;
};

Mealy ReadMealy(const string& inFileName)
{
	Mealy mealy;

	// reading states of mealy
	ifstream input(inFileName);
	string line;
	getline(input, line);
	istringstream ss(line);
	string state;
	getline(ss, state, DELIMETER);
	while (getline(ss, state, DELIMETER))
	{
		mealy.statesWithTransitions.push_back({ state, vector<MealyTransition>() });
	}

	// reading entries and transitions of mealy
	while (getline(input, line))
	{
		// reading entries of mealy
		stringstream ss1(line);
		string entry;
		getline(ss1, entry, DELIMETER);
		mealy.entries.push_back(entry);

		// reading transitions of mealy
		size_t i = 0;
		string transition;
		while (getline(ss1, transition, DELIMETER))
		{
			size_t pos = transition.find(SLASH);
			std::string transitionState = transition.substr(0, pos);
			std::string transitionOut = transition.substr(pos + 1, transition.size());
			mealy.statesWithTransitions[i].transitions.push_back({ transitionState, transitionOut });
			i++;
		}
	}

	return mealy;
}

void WriteMealy(const string& outFileName, const Mealy& mealy)
{
	ofstream output(outFileName);

	// writing states of mealy
	for (const auto& state : mealy.statesWithTransitions)
	{
		output << DELIMETER << state.currentState;
	}
	output << endl;

	// writing entries and transitions of mealy
	for (size_t i = 0; i < mealy.entries.size(); i++)
	{
		// writing entry
		output << mealy.entries[i] << DELIMETER;

		// writing transitions
		for (size_t j = 0; j < mealy.statesWithTransitions.size(); j++)
		{
			output << mealy.statesWithTransitions[j].transitions[i].state << SLASH << mealy.statesWithTransitions[j].transitions[i].out;
			if (j != mealy.statesWithTransitions.size() - 1)
			{
				output << DELIMETER;
			}
		}
		output << endl;
	}
}

Moore ReadMoore(const string& inFileName)
{
	Moore moore;

	ifstream input(inFileName);

	// reading output signals of moore
	string outs;
	getline(input, outs);
	istringstream outSs(outs);
	string out;
	getline(outSs, out, DELIMETER);

	// reading states of moore
	string states;
	getline(input, states);
	istringstream stateSs(states);
	string state;
	getline(stateSs, state, DELIMETER);

	// pushing the outs and states
	while (getline(outSs, out, DELIMETER) && getline(stateSs, state, DELIMETER))
	{
		moore.statesWithTransitions.push_back({ state, out, vector<string>() });
	}

	// reading transition of moore
	string line;
	while (getline(input, line))
	{
		// reading entries of mealy
		stringstream ss(line);
		string entry;
		getline(ss, entry, DELIMETER);
		moore.entries.push_back(entry);

		// reading transitions of mealy
		size_t i = 0;
		string transition;
		while (getline(ss, transition, DELIMETER))
		{
			moore.statesWithTransitions[i].transitions.push_back(transition);
			i++;
		}
	}

	return moore;
}

void WriteMoore(const string& outFileName, const Moore& moore)
{
	ofstream output(outFileName);

	// writing output signals of moore
	for (const auto& stateWithTransition : moore.statesWithTransitions)
	{
		output << DELIMETER << stateWithTransition.out;
	}
	output << endl;

	// writing states of moore
	for (const auto& stateWithTransition : moore.statesWithTransitions)
	{
		output << DELIMETER << stateWithTransition.currentState;
	}
	output << endl;

	// writing transitions of moore
	for (size_t i = 0; i < moore.entries.size(); i++)
	{
		// writing entry
		output << moore.entries[i] << DELIMETER;

		// writing transitions
		for (size_t j = 0; j < moore.statesWithTransitions.size(); j++)
		{
			output << moore.statesWithTransitions[j].transitions[i];
			if (j != moore.statesWithTransitions.size() - 1)
			{
				output << DELIMETER;
			}
		}
		output << endl;
	}
}

Mealy DeleteUnreachableStates(const Mealy& mealy)
{
	Mealy mealyWithoutUnreachableStates;
	mealyWithoutUnreachableStates.entries = mealy.entries;

	unordered_set<string> reachableStates;
	queue<string> queue;
	queue.push(mealy.statesWithTransitions[0].currentState);
	reachableStates.insert(mealy.statesWithTransitions[0].currentState);

	while (!queue.empty())
	{
		string currentState = queue.front();
		queue.pop();

		for (size_t i = 0; i < mealy.statesWithTransitions.size(); i++)
		{
			if (currentState == mealy.statesWithTransitions[i].currentState)
			{
				for (size_t j = 0; j < mealy.statesWithTransitions[i].transitions.size(); j++)
				{
					if (!reachableStates.count(mealy.statesWithTransitions[i].transitions[j].state))
					{
						queue.push(mealy.statesWithTransitions[i].transitions[j].state);
						reachableStates.insert(mealy.statesWithTransitions[i].transitions[j].state);
					}
				}
				break;
			}
		}
	}

	for (const auto& reachableState : reachableStates)
	{
		for (size_t i = 0; i < mealy.statesWithTransitions.size(); i++)
		{
			if (reachableState == mealy.statesWithTransitions[i].currentState)
			{
				mealyWithoutUnreachableStates.statesWithTransitions.push_back(mealy.statesWithTransitions[i]);
				break;
			}
		}
	}

	return mealyWithoutUnreachableStates;
}

Moore DeleteUnreachableStates(const Moore& moore)
{
	Moore mooreWithoutUnreachableStates;
	mooreWithoutUnreachableStates.entries = moore.entries;

	unordered_set<string> reachableStates;
	queue<string> queue;
	queue.push(moore.statesWithTransitions[0].currentState);
	reachableStates.insert(moore.statesWithTransitions[0].currentState);

	while (!queue.empty())
	{
		string currentState = queue.front();
		queue.pop();

		for (size_t i = 0; i < moore.statesWithTransitions.size(); i++)
		{
			if (currentState == moore.statesWithTransitions[i].currentState)
			{
				for (size_t j = 0; j < moore.statesWithTransitions[i].transitions.size(); j++)
				{
					if (!reachableStates.count(moore.statesWithTransitions[i].transitions[j]))
					{
						queue.push(moore.statesWithTransitions[i].transitions[j]);
						reachableStates.insert(moore.statesWithTransitions[i].transitions[j]);
					}
				}
				break;
			}
		}
	}

	for (const auto& reachableState : reachableStates)
	{
		for (size_t i = 0; i < moore.statesWithTransitions.size(); i++)
		{
			if (reachableState == moore.statesWithTransitions[i].currentState)
			{
				mooreWithoutUnreachableStates.statesWithTransitions.push_back(moore.statesWithTransitions[i]);
				break;
			}
		}
	}

	return mooreWithoutUnreachableStates;
}

Moore Convert(const Mealy& mealy)
{
	Moore result;
	Mealy mealyWithoutUnreachableStates = DeleteUnreachableStates(mealy);

	// extracting states for moore
	std::unordered_set<MealyTransition> statesForMoore;
	for (const auto& stateWithTransitions : mealyWithoutUnreachableStates.statesWithTransitions)
	{
		for (const auto& transition : stateWithTransitions.transitions)
		{
			statesForMoore.insert(transition);
		}
	}

	// checking if start state is missing
	bool hasStartState = false;
	for (const auto& state : statesForMoore)
	{
		if (state.state == mealyWithoutUnreachableStates.statesWithTransitions[0].currentState)
		{
			hasStartState = true;
		}
	}
	if (!hasStartState)
	{
		statesForMoore.insert(MealyTransition{ mealyWithoutUnreachableStates.statesWithTransitions[0].currentState, "" });
	}

	// coping moore entries
	result.entries = mealyWithoutUnreachableStates.entries;

	// paste moore states and outs, also coping states in vector
	size_t i = 0;
	vector<MealyTransition> statesForMooreVec;
	for (const auto& state : statesForMoore)
	{
		result.statesWithTransitions.push_back({ CHAR_STATE_FOR_MOORE + to_string(i), state.out, vector<string>(result.entries.size()) });
		statesForMooreVec.push_back(state);
		i++;
	}

	// paste transition for moore
	for (size_t i = 0; i < statesForMooreVec.size(); i++)
	{
		for (size_t j = 0; j < mealy.statesWithTransitions.size(); j++)
		{
			if (statesForMooreVec[i].state == mealy.statesWithTransitions[j].currentState)
			{
				for (size_t k = 0; k < mealy.statesWithTransitions[j].transitions.size(); k++)
				{
					for (size_t p = 0; p < statesForMooreVec.size(); p++)
					{
						if (statesForMooreVec[p] == mealy.statesWithTransitions[j].transitions[k])
						{
							result.statesWithTransitions[i].transitions[k] = result.statesWithTransitions[p].currentState;
							break;
						}
					}
				}
			}
		}
	}

	return result;
}

Mealy Convert(const Moore& moore)
{
	Mealy result;
	Moore mooreWithoutUnreachableStates = DeleteUnreachableStates(moore);

	// coping entries
	result.entries = mooreWithoutUnreachableStates.entries;

	// paste mealy states
	for (const auto& stateWithTransition : mooreWithoutUnreachableStates.statesWithTransitions)
	{
		result.statesWithTransitions.push_back({ stateWithTransition.currentState, vector<MealyTransition>(result.entries.size()) });
	}

	// paste mealy transitions
	for (size_t i = 0; i < mooreWithoutUnreachableStates.statesWithTransitions.size(); i++)
	{
		for (const auto& transition : mooreWithoutUnreachableStates.statesWithTransitions[i].transitions)
		{
			for (const auto& state : mooreWithoutUnreachableStates.statesWithTransitions)
			{
				if (state.currentState == transition)
				{
					result.statesWithTransitions[i].transitions.push_back({ state.currentState, state.out });
				}
			}
		}
	}

	return result;
}

void ConvertToMoore(const string& inFileName, const string& outFileName)
{
	Mealy mealy = ReadMealy(inFileName);
	Moore moore = Convert(mealy);
	WriteMoore(outFileName, moore);
}

void ConvertToMealy(const string& inFileName, const string& outFileName)
{
	Moore moore = ReadMoore(inFileName);
	Mealy mealy = Convert(moore);
	WriteMealy(outFileName, mealy);
}

void WriteBadRequest(const string& message)
{
	cout << message << endl;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		cout << "Usage: " << argv[0] << " <conversion-type> <input.csv> <output.csv>" << endl;
		return 1;
	}

	string convType = argv[1];
	string inputFileName = argv[2];
	string outputFileName = argv[3];

	(convType == CONVERSION_TYPE_MEALY_TO_MOORE) ?
		ConvertToMoore(inputFileName, outputFileName) :
		((convType == CONVERSION_TYPE_MOORE_TO_MEALY) ?
			ConvertToMealy(inputFileName, outputFileName) :
			WriteBadRequest("Invalid type of conversion"));
	return 0;
}
