#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <queue>
#include <unordered_map>

const std::string MEALY_AUTOMATA = "mealy";
const std::string MOORE_AUTOMATA = "moore";
const char DELIMETER = ';';
const char SLASH = '/';

namespace std
{
	template<>
	struct hash<std::vector<std::string>>
	{
		size_t operator()(const std::vector<std::string>& vec) const
		{
			std::hash<std::string> hasher;
			size_t seed = vec.size();
			for (const auto& str : vec)
			{
				seed ^= hasher(str) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
}

namespace std {
	// Хеш-функция для std::vector<size_t>
	template<>
	struct hash<std::vector<size_t>> {
		size_t operator()(const std::vector<size_t>& vec) const {
			std::hash<size_t> hasher;
			size_t seed = vec.size();
			for (const auto& elem : vec) {
				seed ^= hasher(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};

	// Специализация хеш-функции для std::pair<std::vector<size_t>, size_t>
	template<>
	struct hash<std::pair<std::vector<size_t>, size_t>>
	{
		size_t operator()(const std::pair<std::vector<size_t>, size_t>& p) const
		{
			auto hash1 = hash<std::vector<size_t>>{}(p.first);
			auto hash2 = hash<size_t>{}(p.second);
			return hash1 ^ (hash2 << 1); // Объединение хешей
		}
	};
}

namespace std
{
	// Специализация хеш-функции для std::pair<std::vector<std::string>, size_t>
	template<>
	struct hash<std::pair<std::vector<std::string>, size_t>>
	{
		size_t operator()(const std::pair<std::vector<std::string>, size_t>& p) const
		{
			auto hash1 = hash<std::vector<std::string>>{}(p.first);
			auto hash2 = hash<size_t>{}(p.second);
			return hash1 ^ (hash2 << 1); // Объединение хешей
		}
	};
}

struct MealyStateWithTransitions
{
	std::string currentState;
	std::vector<std::string> transitions;
	std::vector<std::string> outs;
};

struct Mealy
{
	std::vector<std::string> entries;
	std::vector<MealyStateWithTransitions> statesWithTransitions;
};

struct MooreStateWithTransitions
{
	std::string currentState;
	std::string out;
	std::vector<std::string> transitions;
};

struct Moore
{
	std::vector<std::string> entries;
	std::vector<MooreStateWithTransitions> statesWithTransitions;
};

Mealy ReadMealy(const std::string& inFileName)
{
	Mealy mealy;

	// reading states of mealy
	std::ifstream input(inFileName);
	std::string line;
	std::getline(input, line);
	std::istringstream ss(line);
	std::string state;
	std::getline(ss, state, DELIMETER);
	while (std::getline(ss, state, DELIMETER))
	{
		mealy.statesWithTransitions.push_back({ state, std::vector<std::string>(), std::vector<std::string>() });
	}

	// reading entries and transitions of mealy
	while (std::getline(input, line))
	{
		// reading entries of mealy
		std::stringstream ss1(line);
		std::string entry;
		std::getline(ss1, entry, DELIMETER);
		mealy.entries.push_back(entry);

		// reading transitions of mealy
		size_t i = 0;
		std::string transition;
		while (std::getline(ss1, transition, DELIMETER))
		{
			size_t pos = transition.find(SLASH);
			std::string transitionState = transition.substr(0, pos);
			std::string transitionOut = transition.substr(pos + 1, transition.size());
			mealy.statesWithTransitions[i].transitions.push_back(transitionState);
			mealy.statesWithTransitions[i].outs.push_back(transitionOut);
			i++;
		}
	}

	return mealy;
}

void WriteMealy(const Mealy& mealy, const std::string& outFileName)
{
	std::ofstream output(outFileName);

	// writing states of mealy
	for (const auto& state : mealy.statesWithTransitions)
	{
		output << DELIMETER << state.currentState;
	}
	output << std::endl;

	// writing entries and transitions of mealy
	for (size_t i = 0; i < mealy.entries.size(); i++)
	{
		// writing entry
		output << mealy.entries[i] << DELIMETER;

		// writing transitions
		for (size_t j = 0; j < mealy.statesWithTransitions.size(); j++)
		{
			output << mealy.statesWithTransitions[j].transitions[i] << SLASH << mealy.statesWithTransitions[j].outs[i];
			if (j != mealy.statesWithTransitions.size() - 1)
			{
				output << DELIMETER;
			}
		}
		output << std::endl;
	}
}

Moore ReadMoore(const std::string& inFileName)
{
	Moore moore;

	std::ifstream input(inFileName);

	// reading output signals of moore
	std::string outs;
	std::getline(input, outs);
	std::istringstream outSs(outs);
	std::string out;
	std::getline(outSs, out, DELIMETER);

	// reading states of moore
	std::string states;
	std::getline(input, states);
	std::istringstream stateSs(states);
	std::string state;
	std::getline(stateSs, state, DELIMETER);

	// pushing the outs and states
	while (std::getline(outSs, out, DELIMETER) && std::getline(stateSs, state, DELIMETER))
	{
		moore.statesWithTransitions.push_back({ state, out, std::vector<std::string>() });
	}

	// reading transition of moore
	std::string line;
	while (std::getline(input, line))
	{
		// reading entries of mealy
		std::istringstream ss(line);
		std::string entry;
		std::getline(ss, entry, DELIMETER);
		moore.entries.push_back(entry);

		// reading transitions of mealy
		size_t i = 0;
		std::string transition;
		while (std::getline(ss, transition, DELIMETER))
		{
			moore.statesWithTransitions[i].transitions.push_back(transition);
			i++;
		}
	}

	return moore;
}

void WriteMoore(const Moore& moore, const std::string& outFileName)
{
	std::ofstream output(outFileName);

	// writing output signals of moore
	for (const auto& stateWithTransition : moore.statesWithTransitions)
	{
		output << DELIMETER << stateWithTransition.out;
	}
	output << std::endl;

	// writing states of moore
	for (const auto& stateWithTransition : moore.statesWithTransitions)
	{
		output << DELIMETER << stateWithTransition.currentState;
	}
	output << std::endl;

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
		output << std::endl;
	}
}

Mealy DeleteUnreachableStates(const Mealy& mealy)
{
	Mealy mealyWithoutUnreachableStates;
	mealyWithoutUnreachableStates.entries = mealy.entries;

	std::unordered_set<std::string> reachableStates;
	std::queue<std::string> queue;
	queue.push(mealy.statesWithTransitions[0].currentState);
	reachableStates.insert(mealy.statesWithTransitions[0].currentState);

	while (!queue.empty())
	{
		std::string currentState = queue.front();
		queue.pop();

		for (size_t i = 0; i < mealy.statesWithTransitions.size(); i++)
		{
			if (currentState == mealy.statesWithTransitions[i].currentState)
			{
				for (size_t j = 0; j < mealy.statesWithTransitions[i].transitions.size(); j++)
				{
					if (!reachableStates.count(mealy.statesWithTransitions[i].transitions[j]))
					{
						queue.push(mealy.statesWithTransitions[i].transitions[j]);
						reachableStates.insert(mealy.statesWithTransitions[i].transitions[j]);
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

	std::unordered_set<std::string> reachableStates;
	std::queue<std::string> queue;
	queue.push(moore.statesWithTransitions[0].currentState);
	reachableStates.insert(moore.statesWithTransitions[0].currentState);

	while (!queue.empty())
	{
		std::string currentState = queue.front();
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

void MinimizeMealy(const std::string& inFileName, const std::string& outFileName)
{
	Mealy mealy = ReadMealy(inFileName);
	mealy = DeleteUnreachableStates(mealy);

	// minimization
	std::unordered_map<std::vector<std::string>, std::vector<MealyStateWithTransitions>> mapOfGroups;
	for (const auto& state : mealy.statesWithTransitions)
	{
		mapOfGroups[state.outs].push_back(state);
	}
	std::unordered_multimap<size_t, MealyStateWithTransitions> multimapOfGroups;
	size_t number = 0;
	for (const auto& pair : mapOfGroups)
	{
		for (const auto& state : pair.second)
		{
			multimapOfGroups.insert({ number, state });
		}
		number++;
	}

	std::unordered_multimap<size_t, std::pair<std::vector<size_t>, MealyStateWithTransitions>> transits;
	for (const auto& pair : multimapOfGroups)
	{
		std::vector<size_t> transitsForOneState;
		for (const auto& transit : pair.second.transitions)
		{
			for (const auto& newPair : multimapOfGroups)
			{
				if (transit == newPair.second.currentState)
				{
					transitsForOneState.push_back(newPair.first);
				}
			}
		}
		transits.insert({ pair.first, std::make_pair(transitsForOneState, pair.second) });
	}

	size_t prevSize = mapOfGroups.size();
	size_t currSize = 0;
	std::unordered_map<std::pair<std::vector<size_t>, size_t>, std::vector<MealyStateWithTransitions>> finalMapOfGroups; // key - {transits, number of group}, value - vector of states in this group
	std::unordered_multimap<size_t, MealyStateWithTransitions> finalMultimap; // key - num of group, value - state with transits
	std::unordered_multimap<size_t, std::pair<std::vector<size_t>, MealyStateWithTransitions>> finalTransits; // key - num of group, value - {vector of new transits, old state with old transits
	while (true)
	{
		std::unordered_map<std::pair<std::vector<size_t>, size_t>, std::vector<MealyStateWithTransitions>> newMapOfGroups;
		std::unordered_multimap<size_t, MealyStateWithTransitions> newMultimap;
		std::unordered_multimap<size_t, std::pair<std::vector<size_t>, MealyStateWithTransitions>> newTransits;

		for (const auto& pair : transits)
		{
			newMapOfGroups[{pair.second.first, pair.first}].push_back(pair.second.second);
		}
		size_t newNumber = 0;
		for (const auto& pair : newMapOfGroups)
		{
			for (const auto& state : pair.second)
			{
				newMultimap.insert({ newNumber, state });
			}
			newNumber++;
		}

		for (const auto& pair : newMultimap)
		{
			std::vector<size_t> transitsForOneState;
			for (const auto& transit : pair.second.transitions)
			{
				for (const auto& newPair : newMultimap)
				{
					if (transit == newPair.second.currentState)
					{
						transitsForOneState.push_back(newPair.first);
					}
				}
			}
			newTransits.insert({ pair.first, std::make_pair(transitsForOneState, pair.second) });
		}

		currSize = newMapOfGroups.size();
		prevSize = mapOfGroups.size();

		if (currSize == prevSize)
		{
			finalMultimap = newMultimap;
			finalMapOfGroups = newMapOfGroups;
			finalTransits = newTransits;
			break;
		}
		else
		{
			multimapOfGroups = newMultimap;
			transits = newTransits;
		}
	}

	Mealy minMealy;
	minMealy.entries = mealy.entries;
	std::vector<std::vector<MealyStateWithTransitions>> states(finalMapOfGroups.size());
	for (const auto& pair : finalMultimap)
	{
		states[pair.first].push_back(pair.second);
	}
	std::vector<std::vector<std::string>> minTransits(states.size());
	for (const auto& pair : finalTransits)
	{
		std::vector<std::string> minT;
		for (const auto& t : pair.second.first)
		{
			minT.push_back("X" + std::to_string(t));
		}
		minTransits[pair.first] = minT;
	}
	for (size_t j = 0; j < states.size(); j++)
	{
		minMealy.statesWithTransitions.push_back({ "X" + std::to_string(j), std::vector<std::string>(), std::vector<std::string>() });
	}
	for (size_t j = 0; j < states.size(); j++)
	{
		minMealy.statesWithTransitions[j].outs = states[j][0].outs;
		minMealy.statesWithTransitions[j].transitions = minTransits[j];
	}

	WriteMealy(minMealy, outFileName);
}

void MinimizeMoore(const std::string& inFileName, const std::string& outFileName)
{
	Moore moore = ReadMoore(inFileName);
	moore = DeleteUnreachableStates(moore);

	// minimization
	std::unordered_map<std::string, std::vector<MooreStateWithTransitions>> mapOfGroups;
	for (const auto& state : moore.statesWithTransitions)
	{
		mapOfGroups[state.out].push_back(state);
	}
	std::unordered_multimap<size_t, MooreStateWithTransitions> multimapOfGroups;
	size_t number = 0;
	for (const auto& pair : mapOfGroups)
	{
		for (const auto& state : pair.second)
		{
			multimapOfGroups.insert({ number, state });
		}
		number++;
	}

	std::unordered_multimap<size_t, std::pair<std::vector<size_t>, MooreStateWithTransitions>> transits;
	for (const auto& pair : multimapOfGroups)
	{
		std::vector<size_t> transitsForOneState;
		for (const auto& transit : pair.second.transitions)
		{
			for (const auto& newPair : multimapOfGroups)
			{
				if (transit == newPair.second.currentState)
				{
					transitsForOneState.push_back(newPair.first);
				}
			}
		}
		transits.insert({ pair.first, std::make_pair(transitsForOneState, pair.second) });
	}

	size_t prevSize = mapOfGroups.size();
	size_t currSize = 0;
	std::unordered_map<std::pair<std::vector<size_t>, size_t>, std::vector<MooreStateWithTransitions>> finalMapOfGroups; // key - {transits, number of group}, value - vector of states in this group
	std::unordered_multimap<size_t, MooreStateWithTransitions> finalMultimap; // key - num of group, value - state with transits
	std::unordered_multimap<size_t, std::pair<std::vector<size_t>, MooreStateWithTransitions>> finalTransits; // key - num of group, value - {vector of new transits, old state with old transits
	while (true)
	{
		std::unordered_map<std::pair<std::vector<size_t>, size_t>, std::vector<MooreStateWithTransitions>> newMapOfGroups;
		std::unordered_multimap<size_t, MooreStateWithTransitions> newMultimap;
		std::unordered_multimap<size_t, std::pair<std::vector<size_t>, MooreStateWithTransitions>> newTransits;

		for (const auto& pair : transits)
		{
			newMapOfGroups[{pair.second.first, pair.first}].push_back(pair.second.second);
		}
		size_t newNumber = 0;
		for (const auto& pair : newMapOfGroups)
		{
			for (const auto& state : pair.second)
			{
				newMultimap.insert({ newNumber, state });
			}
			newNumber++;
		}

		for (const auto& pair : newMultimap)
		{
			std::vector<size_t> transitsForOneState;
			for (const auto& transit : pair.second.transitions)
			{
				for (const auto& newPair : newMultimap)
				{
					if (transit == newPair.second.currentState)
					{
						transitsForOneState.push_back(newPair.first);
					}
				}
			}
			newTransits.insert({ pair.first, std::make_pair(transitsForOneState, pair.second) });
		}

		currSize = newMapOfGroups.size();
		prevSize = mapOfGroups.size();

		if (currSize == prevSize)
		{
			finalMultimap = newMultimap;
			finalMapOfGroups = newMapOfGroups;
			finalTransits = newTransits;
			break;
		}
		else
		{
			multimapOfGroups = newMultimap;
			transits = newTransits;
		}
	}

	Moore minMoore;
	minMoore.entries = moore.entries;
	std::vector<std::vector<MooreStateWithTransitions>> states(finalMapOfGroups.size());
	for (const auto& pair : finalMultimap)
	{
		states[pair.first].push_back(pair.second);
	}
	std::vector<std::vector<std::string>> minTransits(states.size());
	for (const auto& pair : finalTransits)
	{
		std::vector<std::string> minT;
		for (const auto& t : pair.second.first)
		{
			minT.push_back("X" + std::to_string(t));
		}
		minTransits[pair.first] = minT;
	}
	for (size_t j = 0; j < states.size(); j++)
	{
		minMoore.statesWithTransitions.push_back({ "X" + std::to_string(j), states[j][0].out, std::vector<std::string>() });
	}
	for (size_t j = 0; j < states.size(); j++)
	{
		minMoore.statesWithTransitions[j].transitions = minTransits[j];
	}

	WriteMoore(minMoore, outFileName);
}

void WriteBadRequest(const std::string& message)
{
	std::cout << message << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage: " << argv[0] << " <type-of-automata> <input.csv> <output.csv>" << std::endl;
		return 1;
	}

	std::string automataType = argv[1];
	std::string inputFileName = argv[2];
	std::string outputFileName = argv[3];

	(automataType == MEALY_AUTOMATA) ?
		MinimizeMealy(inputFileName, outputFileName) :
		((automataType == MOORE_AUTOMATA) ?
			MinimizeMoore(inputFileName, outputFileName) :
			WriteBadRequest("Invalid type of automata"));
	return 0;
}