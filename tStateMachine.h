#pragma once

class tPetrelState {
public:
	enum class St {
		Init,
		Inited,

	};
private:
	St State;

public:
	St GetState() const { return State; }
	void SetState(St st) { State = st; }
};
