#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <unordered_set>
#include <queue>

const std::string CONVERSION_TYPE_MEALY_TO_MOORE = "mealy-to-moore";
const std::string CONVERSION_TYPE_MOORE_TO_MEALY = "moore-to-mealy";

struct Mealy
{
	std::vector<std::string> states;
	std::vector<std::string> entries;
	std::vector<std::vector<std::pair<std::string, std::string>>> transitions;
};

struct Moore
{
	std::vector<std::pair<std::string, std::string>> states;
	std::vector<std::string> entries;
	std::vector<std::vector<std::string>> transitions;
};

void ReadMealy(const std::string& inFileName, Mealy& mealy)
{
	std::ifstream input(inFileName);

	// чтение состояний
	std::string line;
	std::getline(input, line);
	std::stringstream ss(line);
	std::string state;
	std::getline(ss, state, ';');

	while (std::getline(ss, state, ';'))
	{
		mealy.states.push_back(state);
	}

	while (std::getline(input, line))
	{
		// чтение входных сигналов
		std::stringstream ss(line);
		std::string entry;
		std::getline(ss, entry, ';');
		mealy.entries.push_back(entry);

		// чтение переходов
		std::string transition;
		std::vector<std::pair<std::string, std::string>> transitions;
		for (const auto& state : mealy.states)
		{
			std::getline(ss, transition, ';');
			size_t pos = transition.find('/');
			std::string transitionState = transition.substr(0, pos);
			std::string transitionOut = transition.substr(pos + 1, transition.size());
			transitions.push_back(std::make_pair(transitionState, transitionOut));
		}
		mealy.transitions.push_back(transitions);
	}
}

void WriteMealy(const std::string& outFileName, Mealy mealy)
{
	std::ofstream output(outFileName);

	for (const auto& state : mealy.states)
	{
		output << ";" << state;
	}
	output << std::endl;

	for (int i = 0; i < mealy.transitions.size(); i++)
	{
		output << mealy.entries[i] << ";";
		for (int j = 0; j < mealy.transitions[i].size(); j++)
		{
			output << mealy.transitions[i][j].first
				<< "/" << mealy.transitions[i][j].second;
			if (j != mealy.transitions[i].size() - 1)
			{
				output << ";";
			}
		}
		output << std::endl;
	}
}

void ReadMoore(const std::string& inFileName, Moore& moore)
{
	std::ifstream input(inFileName);

	// чтение состояний
	std::string outString;
	std::string stateString;

	std::getline(input, outString);
	std::getline(input, stateString);

	std::stringstream ssOut(outString);
	std::stringstream ssState(stateString);

	std::string out;
	std::getline(ssOut, out, ';');
	std::string state;
	std::getline(ssState, state, ';');

	while (std::getline(ssOut, out, ';') && std::getline(ssState, state, ';'))
	{
		moore.states.push_back(std::make_pair(state, out));
	}

	std::string line;
	while (std::getline(input, line))
	{
		// чтение входных сигналов
		std::stringstream ss(line);
		std::string entry;
		std::getline(ss, entry, ';');
		moore.entries.push_back(entry);

		// чтение переходов
		std::string transition;
		std::vector<std::string> transitions;
		for (const auto& state : moore.states)
		{
			std::getline(ss, transition, ';');
			transitions.push_back(transition);
		}
		moore.transitions.push_back(transitions);
	}
}

void WriteMoore(const std::string& outFileName, Moore moore)
{
	std::ofstream output(outFileName);

	for (const auto& state : moore.states)
	{
		output << ";" << state.second;
	}
	output << std::endl;
	for (const auto& state : moore.states)
	{
		output << ";" << state.first;
	}
	output << std::endl;

	for (int i = 0; i < moore.transitions.size(); i++)
	{
		output << moore.entries[i] << ";";
		for (int j = 0; j < moore.transitions[i].size(); j++)
		{
			output << moore.transitions[i][j];
			if (j != moore.transitions[i].size() - 1)
			{
				output << ";";
			}
		}
		output << std::endl;
	}
}

std::vector<std::pair<std::string, std::string>> ExtractMooreStates(const std::vector<std::vector<std::pair<std::string, std::string>>>& mealyTransitions, const std::string& startState)
{
	std::set<std::pair<std::string, std::string>> statesForMoore;
	for (const auto& transitionForOneEntry : mealyTransitions)
	{
		for (const auto& transition : transitionForOneEntry)
		{
			statesForMoore.insert(transition);
		}
	}

	std::vector<std::pair<std::string, std::string>> statesForMooreVector;
	bool containsStartState = false;
	for (const auto& state : statesForMoore)
	{
		statesForMooreVector.push_back(state);
		if (state.first == startState)
		{
			containsStartState = true;
		}
	}

	if (!containsStartState)
	{
		statesForMooreVector.insert(statesForMooreVector.begin(), std::make_pair(startState, ""));
	}

	return statesForMooreVector;
}

std::unordered_set<int> FindReachableStates(const Mealy& mealy)
{
	std::unordered_set<std::string> reachableStates;
	std::queue<std::string> queue;
	queue.push(mealy.states[0]);
	reachableStates.insert(mealy.states[0]);

	while (!queue.empty())
	{
		std::string currentState = queue.front();
		queue.pop();

		for (int i = 0; i < mealy.states.size(); i++)
		{
			if (currentState == mealy.states[i])
			{
				for (int j = 0; j < mealy.entries.size(); j++)
				{
					if (!reachableStates.count(mealy.transitions[j][i].first))
					{
						queue.push(mealy.transitions[j][i].first);
						reachableStates.insert(mealy.transitions[j][i].first);
					}
				}
				break;
			}
		}
	}

	std::unordered_set<int> reachableStatesIndexes;
	for (const auto& reachableState : reachableStates)
	{
		for (int i = 0; i < mealy.states.size(); i++)
		{
			if (reachableState == mealy.states[i])
			{
				reachableStatesIndexes.insert(i);
				break;
			}
		}
	}

	return reachableStatesIndexes;
}

std::unordered_set<int> FindReachableStates(const Moore& moore)
{
	std::unordered_set<std::string> reachableStates;
	std::queue<std::string> queue;
	queue.push(moore.states[0].first);
	reachableStates.insert(moore.states[0].first);

	while (!queue.empty())
	{
		std::string currentState = queue.front();
		queue.pop();

		for (int i = 0; i < moore.states.size(); i++)
		{
			if (currentState == moore.states[i].first)
			{
				for (int j = 0; j < moore.entries.size(); j++)
				{
					if (!reachableStates.count(moore.transitions[j][i]))
					{
						queue.push(moore.transitions[j][i]);
						reachableStates.insert(moore.transitions[j][i]);
					}
				}
				break;
			}
		}
	}

	std::unordered_set<int> reachableStatesIndexes;
	for (const auto& reachableState : reachableStates)
	{
		for (int i = 0; i < moore.states.size(); i++)
		{
			if (reachableState == moore.states[i].first)
			{
				reachableStatesIndexes.insert(i);
				break;
			}
		}
	}

	return reachableStatesIndexes;
}

void ConvertToMoore(const std::string& inFileName, const std::string& outFileName)
{
	Mealy mealy;
	ReadMealy(inFileName, mealy);
	Moore moore;

	std::unordered_set<int> reachableStates = FindReachableStates(mealy);
	Mealy filteredMealy;
	filteredMealy.entries = mealy.entries;
	filteredMealy.transitions = std::vector<std::vector<std::pair<std::string, std::string>>>(filteredMealy.entries.size());
	for (const auto& index : reachableStates)
	{
		filteredMealy.states.push_back(mealy.states[index]);
		for (int i = 0; i < filteredMealy.entries.size(); i++)
		{
			filteredMealy.transitions[i].push_back(mealy.transitions[i][index]);
		}
	}

	auto statesForMoore = ExtractMooreStates(filteredMealy.transitions, filteredMealy.states[0]);

	// states for moore
	for (int i = 0; i < statesForMoore.size(); i++)
	{
		moore.states.push_back(std::make_pair("q" + std::to_string(i), statesForMoore[i].second));
	}

	// entries for moore
	moore.entries = filteredMealy.entries;

	// transitions for moore
	std::vector<std::vector<std::string>> transitionsMoore(filteredMealy.transitions.size(),
		std::vector<std::string>(filteredMealy.transitions[0].size()));

	for (int i = 0; i < filteredMealy.transitions.size(); i++)
	{
		for (int j = 0; j < filteredMealy.transitions[0].size(); j++)
		{
			int k = 0;
			auto transition = filteredMealy.transitions[i][j];

			for (const auto& state : statesForMoore)
			{
				if (transition == state)
				{
					transitionsMoore[i][j] = moore.states[k].first;
				}
				k++;
			}
		}
	}

	std::vector<std::vector<std::string>> transitions(filteredMealy.transitions.size(),
		std::vector<std::string>(statesForMoore.size()));

	for (int i = 0; i < moore.states.size(); i++)
	{
		for (int j = 0; j < moore.entries.size(); j++)
		{
			for (int k = 0; k < filteredMealy.states.size(); k++)
			{
				if (statesForMoore[i].first == filteredMealy.states[k])
				{
					transitions[j][i] = transitionsMoore[j][k];
				}
			}
		}
	}

	moore.transitions = transitions;

	WriteMoore(outFileName, moore);
}

void ConvertToMealy(const std::string& inFileName, const std::string& outFileName)
{
	Moore moore;
	ReadMoore(inFileName, moore);
	Mealy mealy;

	std::unordered_set<int> reachableStates = FindReachableStates(moore);
	Moore filteredMoore;
	filteredMoore.entries = moore.entries;
	filteredMoore.transitions = std::vector<std::vector<std::string>>(filteredMoore.entries.size());
	for (const auto& index : reachableStates)
	{
		filteredMoore.states.push_back(moore.states[index]);
		for (int i = 0; i < filteredMoore.entries.size(); i++)
		{
			filteredMoore.transitions[i].push_back(moore.transitions[i][index]);
		}
	}

	for (const auto& state : filteredMoore.states)
	{
		mealy.states.push_back(state.first);
	}

	for (const auto& entry : filteredMoore.entries)
	{
		mealy.entries.push_back(entry);
	}

	for (size_t i = 0; i < filteredMoore.transitions.size(); i++)
	{
		std::vector<std::pair<std::string, std::string>> transitionForOneEntry;
		for (size_t j = 0; j < filteredMoore.transitions[i].size(); j++)
		{
			for (const auto& state : filteredMoore.states)
			{
				if (state.first == filteredMoore.transitions[i][j])
				{
					transitionForOneEntry.push_back(std::make_pair(state.first, state.second));
					break;
				}
			}
		}
		mealy.transitions.push_back(transitionForOneEntry);
	}

	WriteMealy(outFileName, mealy);
}

void WriteBadRequest(const std::string& msg)
{
	std::cout << msg << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage: " << argv[0] << " <conversion-type> <input.csv> <output.csv>" << std::endl;
		return 1;
	}

	std::string convType = argv[1];
	std::string inputFileName = argv[2];
	std::string outputFileName = argv[3];

	(convType == CONVERSION_TYPE_MEALY_TO_MOORE) ?
		ConvertToMoore(inputFileName, outputFileName) :
		((convType == CONVERSION_TYPE_MOORE_TO_MEALY) ?
			ConvertToMealy(inputFileName, outputFileName) :
			WriteBadRequest("Invalid type of conversion"));
	return 0;
}
