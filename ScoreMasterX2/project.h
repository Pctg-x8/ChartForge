#pragma once

#include <string>
#include <memory>
#include <array>
#include <list>
#include <vector>
#include <d2d1.h>

D2D1_COLOR_F makeRandomTrackColor();

class Track
{
	// Track Structure
	std::wstring name;
	bool is_editable;
	bool is_activated;
	D2D1_COLOR_F trackColor;
public:
	Track(const std::wstring& n, bool ie, bool ia, const D2D1_COLOR_F& col);
	virtual ~Track();

	auto getName() { return this->name; }
	auto isEditable() { return this->is_editable; }
	auto isActivated() { return this->is_activated; }
	auto getTrackColor() { return this->trackColor; }
};

struct SongPosition
{
	uint32_t barCount, beatNum, beatDenom;

	SongPosition(uint32_t bc, uint32_t bn, uint32_t bd)
	{
		this->barCount = bc;
		this->beatNum = bn;
		this->beatDenom = bd;
	}
	SongPosition(const SongPosition& ref)
	{
		this->barCount = ref.barCount;
		this->beatNum = ref.beatNum;
		this->beatDenom = ref.beatDenom;
	}
};

struct TempoInfo
{
	// tempo info
	SongPosition pos;
	double tempo;

	TempoInfo(const SongPosition& p, double t) : pos(p)
	{
		this->tempo = t;
	}
};
struct BeatInfo
{
	// beat info
	uint32_t barCount;
	uint32_t beatNum, beatDenom;

	BeatInfo(uint32_t barCount, uint32_t beatNum, uint32_t beatDenom)
	{
		this->barCount = barCount;
		this->beatNum = beatNum;
		this->beatDenom = beatDenom;
	}
};

class Project
{
	// Project Structure
public:
	using SystemTrackListT = std::array<std::shared_ptr<Track>, 3>;
	using UserTrackListT = std::vector<std::shared_ptr<Track>>;
private:
	SystemTrackListT systemTracks;		// for visualization
	UserTrackListT userTracks;

	std::list<TempoInfo> tempoInfos;
	std::list<BeatInfo> beatInfos;

	double qtzDenom;
public:
	Project();
	~Project();

	void addEmptyTrack();

	auto getBeginIterator_SystemTracks() { return std::cbegin(this->systemTracks); }
	auto getEndIterator_SystemTracks() { return std::cend(this->systemTracks); }
	auto getBeginIterator_UserTracks() { return std::cbegin(this->userTracks); }
	auto getEndIterator_UserTracks() { return std::cend(this->userTracks); }
	const auto& getTempoList() const { return this->tempoInfos; }
	const auto& getBeatList() const { return this->beatInfos; }
	auto getQuantizeValue() { return 1.0f / this->qtzDenom; }
	auto getQuantizeValueInv() { return this->qtzDenom; }
};

class ProjectManager
{
	std::wstring filePath;
	std::unique_ptr<Project> pProjectInstance;
public:
	ProjectManager();
	~ProjectManager();

	void createEmpty();
	auto getFilePath() { return this->filePath; }
	auto getCurrent() { return this->pProjectInstance.get(); }
};