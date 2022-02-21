#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

const int GROUPS_PER_SERIES = 4;
const int TEAMS_PER_GROUP = 4;
const int MATCHES_PER_GROUP = TEAMS_PER_GROUP * (TEAMS_PER_GROUP - 1) / 2;
const int MATCHES_PER_SERIES = TEAMS_PER_GROUP * MATCHES_PER_GROUP;


struct MatchScore {
	enum class Group {
		GROUP_A,
		GROUP_B,
		GROUP_C,
		GROUP_D,
		GROUP_COUNT
	};

	std::string team1;
	std::string team2;
	int team1Score = 0;
	int team2Score = 0;
	Group group = Group::GROUP_A;
};


class InputParser {
public:
	bool parseRow(const std::string& line, MatchScore& score)
	{
		auto idx = line.find(' ');
		if (idx == std::string::npos)
			return false;

		score.team1 = line.substr(0, idx);
		++idx;

		auto idx2 = line.find(" : ", idx);
		if (idx2 == std::string::npos || idx2 - idx != 1)
			return false;

		score.team1Score = (int)(line[idx] - '0');
		idx = idx2 + 3;

		idx2 = line.find(' ', idx);
		if (idx2 == std::string::npos || idx2 - idx != 1)
			return false;

		score.team2Score = (int)(line[idx] - '0');
		idx = idx2 + 1;

		score.team2 = line.substr(idx);

		score.group = (MatchScore::Group)((2 * m_rowCount / TEAMS_PER_GROUP) % GROUPS_PER_SERIES);
		++m_rowCount;

		return true;
	}

private:
	int m_rowCount = 0;
};


struct TeamStats {
	void addScore(int scored, int conceded)
	{
		this->scored += scored;
		this->conceded += conceded;

		if (scored > conceded)
			points += 3;
		else if (scored == conceded)
			points += 1;
	}

	std::string name;
	int id = 0;
	int points = 0;
	int scored = 0;
	int conceded = 0;
};


class GroupManager {
public:
	void addScore(const MatchScore& score)
	{
		int id1 = getTeamId(score.team1);
		int id2 = getTeamId(score.team2);

		m_teams[id1].addScore(score.team1Score, score.team2Score);
		m_teams[id2].addScore(score.team2Score, score.team1Score);
		m_scoreMap[id1][id2] = score.team1Score - score.team2Score;
		m_scoreMap[id2][id1] = score.team2Score - score.team1Score;
		m_sorted = false;
	}

	std::pair<std::string, std::string> getBest()
	{
		if (!m_sorted) {
			std::sort(m_teams.begin(), m_teams.end(), [](const TeamStats& a, const TeamStats& b) { return a.points > b.points; });

			int j = 0;
			int upper = (int)m_teams.size();
			for (int i = 1; i <= upper; ++i) {
				if (i == upper || m_teams[j].points != m_teams[i].points) {
					int toSort = i - j;
					if (toSort > 1) {
						std::sort(
							m_teams.begin() + j,
							m_teams.begin() + (j + toSort),
							[this, toSort](const TeamStats& a, const TeamStats& b) { return compareTeamStats(a, b, toSort == 2) > 0; });
					}

					j = i;
				}
			}

			m_sorted = true;

			std::cout << std::endl;
			for (auto& t : m_teams) {
				std::cout << t.name << ' ' << t.points << ' ' << t.scored - t.conceded << ' ' << t.scored << ' ' << t.conceded << std::endl;
			}
			std::cout << std::endl;
		}

		return std::make_pair(m_teams[0].name, m_teams[1].name);
	}

private:
	int getTeamId(const std::string& team)
	{
		for (auto& t : m_teams) {
			if (t.name == team)
				return t.id;
		}

		TeamStats t;
		t.name = team;
		t.id = (int)m_teams.size();
		m_teams.push_back(t);

		return t.id;
	}

	int compareTeamStats(const TeamStats& a, const TeamStats& b, bool compareDirect)
	{
		int ret = 0;

		if (compareDirect) {
			ret = m_scoreMap[a.id][b.id];
			if (ret != 0)
				return ret;
		}

		ret = (a.scored - a.conceded) - (b.scored - b.conceded);
		if (ret != 0)
			return ret;

		ret = a.scored - b.scored;
		if (ret != 0)
			return ret;

		return b.name.compare(a.name);
	}

	std::vector<TeamStats> m_teams;
	int m_scoreMap[TEAMS_PER_GROUP][TEAMS_PER_GROUP] = { 0 };
	bool m_sorted = true;
};


void printOpponents(GroupManager& g1, GroupManager& g2)
{
	auto best1 = g1.getBest();
	auto best2 = g2.getBest();

	std::cout << best1.first << " - " << best2.second << std::endl;
	std::cout << best2.first << " - " << best1.second << std::endl;
}


void processDataSet(std::ifstream& inputFile)
{
	int lineCount = 0;
	std::string line;
	MatchScore score;

	InputParser parser;
	GroupManager groupManagers[GROUPS_PER_SERIES];

	while (lineCount < MATCHES_PER_SERIES && std::getline(inputFile, line)) {
		parser.parseRow(line, score);
		groupManagers[(int)score.group].addScore(score);
		++lineCount;
	}

	for (int i = 0; i < GROUPS_PER_SERIES; i += 2) {
		printOpponents(groupManagers[i], groupManagers[i + 1]);
	}
}


int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout << "source file path not provided" << std::endl;
		return -1;
	}

	const char* filepath = argv[1];
	std::ifstream inputFile(filepath);
	if (!inputFile.good()) {
		std::cout << "cannot open file: " << filepath << std::endl;
		return -2;
	}

	try {
		int dataSetCount = 0;
		inputFile >> dataSetCount;
		inputFile.ignore();

		for (int i = 0; i < dataSetCount; ++i) {
			processDataSet(inputFile);
		}
	}
	catch (...) {
		std::cout << "invalid input file syntax" << std::endl;
		return -3;
	}

	return 0;
}
