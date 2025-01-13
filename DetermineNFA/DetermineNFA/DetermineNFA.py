from queue import Queue
from itertools import chain
import sys


def read_machine_from_csv(file_name):
    try:
        machine = {
            "entries": [],
            "states_with_transitions": []
        }
        with open(file_name, 'r', encoding='utf-8') as file:
            outs = file.readline().split(';')[1:]
            states = file.readline().split(';')[1:]
            for i in range(len(outs)):
                machine["states_with_transitions"].append({"current_state": states[i].replace('\n', ''), "out": outs[i].replace('\n', ''), "transitions": []})

            for line in file:
                entry = line.split(';')[0]
                transits = line.split(';')[1:]
                machine["entries"].append(entry)
                for i in range(len(transits)):
                    tr = transits[i].replace('\n', '').split(',')
                    new_transits = []
                    for t in tr:
                        if t:
                            new_transits.append(t)

                    if new_transits:
                        machine["states_with_transitions"][i]["transitions"].append({"states": transits[i].replace('\n', '').split(','), "entry": entry})

        #print(machine)
        return machine

    except FileNotFoundError:
        print("File not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


def epsilon_closure(current_states, nfa):
    epsilon_filled = []
    q = Queue()
    for state in current_states:
        epsilon_filled.append(state)
        q.put(state)
        while not q.empty():
            st = q.get()
            for transit in st["transitions"]:
                if transit["entry"] == 'ε':
                    for t in transit["states"]:
                        for s in nfa["states_with_transitions"]:
                            if s["current_state"] == t and s not in epsilon_filled:
                                epsilon_filled.append(s)
                                q.put(s)

    #print(epsilon_filled)
    unique_epsilon_filled = []
    for item in epsilon_filled:
        if item not in unique_epsilon_filled:
            unique_epsilon_filled.append(item)
    #print(unique_epsilon_filled)
    unique_epsilon_filled.sort(key=lambda x: x["current_state"])
    #print(unique_epsilon_filled)
    return unique_epsilon_filled #надо преобразовать этот массив в множество чтобы избежать повторов


def determine(nfa):
    dfa = {
        "entries": [],
        "states_with_transitions": []
    }

    for entry in nfa["entries"]:
        if entry != 'ε':
            dfa["entries"].append(entry)

    q = Queue()
    new_state = []
    new_state.append(nfa["states_with_transitions"][0])
    q.put(new_state)
    while not q.empty():
        current_states = q.get()
        # дополняем состояниями, достижимыми из данных только по эпсилон-переходам
        new_state = epsilon_closure(current_states, nfa)

        new_state_str = ''
        out_str = ''
        transitions = []
        for state in new_state:
            new_state_str += state["current_state"]
            if out_str != 'F' and state["out"] == 'F':
                out_str = 'F'

            for t in state["transitions"]:
                if t["entry"] != 'ε':
                    if not transitions:
                        transitions.append({"entry": t["entry"], "states": t["states"]})
                        continue
                    for tr in transitions:
                        if tr["entry"] == t["entry"]:
                            tr["states"] = list(dict.fromkeys(chain(tr["states"], t["states"])))
                            break
                    else:
                        transitions.append({"entry": t["entry"], "states": t["states"]})

        dfa["states_with_transitions"].append({"current_state": new_state_str, "out": out_str, "transitions": []})
        #print(transitions)
        states = []
        for t in transitions:
            for s in t["states"]:
                for state in nfa["states_with_transitions"]:
                    if state["current_state"] == s:
                        states.append(state)

            possible_state = epsilon_closure(states, nfa)
            possible_state_str = ''
            for state in possible_state:
                possible_state_str += state["current_state"]
            t["states"] = possible_state_str
            
            for state in dfa["states_with_transitions"]:
                if state["current_state"] == possible_state_str:
                    break
            else:
                q.put(possible_state)

        dfa["states_with_transitions"][len(dfa["states_with_transitions"]) - 1]["transitions"] = transitions
    
    print(dfa)
    return dfa


def write_machine_to_csv(moore_machine, file_name):
    try:
        with open(file_name, 'w', encoding='utf-8') as file:
            for state in moore_machine["states_with_transitions"]:
                file.write(';' + state["out"])
            file.write('\n')
            for state in moore_machine["states_with_transitions"]:
                file.write(';' + state["current_state"])
            file.write('\n')
            for entry in moore_machine["entries"]:
                file.write(entry + ';')
                for state in moore_machine["states_with_transitions"]:
                    file_transits = []
                    for transit in state["transitions"]:
                        if transit["entry"] == entry:
                            file_transits.append(transit["states"])
                    for t in file_transits:
                        if t != file_transits[len(file_transits) - 1]:
                            file.write(t + ',')
                        else:
                            file.write(t)
                    if state != moore_machine["states_with_transitions"][len(moore_machine["states_with_transitions"]) - 1]:
                        file.write(';')
                file.write('\n')
    except FileNotFoundError:
        print("Файл не найден")
        sys.exit(1)
    except Exception as e:
        print(f"Произошла ошибка: {e}")
        sys.exit(1)


def determine_nfa(in_file_name, out_file_name):
    nfa = read_machine_from_csv(in_file_name)
    dfa = determine(nfa)
    write_machine_to_csv(dfa, out_file_name)


def main():
    if len(sys.argv) != 3:
        print("Usage: ", sys.argv[0], " <input.csv> <output.csv>")
        sys.exit(1)

    input_file_name = sys.argv[1]
    output_file_name = sys.argv[2]

    determine_nfa(input_file_name, output_file_name)
    sys.exit(0)


if __name__ == "__main__":
    main()
