#include "mmu/core/CutsceneRuntime.h"

#include "mmu/core/RegisteredCutscenes.h"
#include "mmu/core/StateDispatcher.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

namespace mmu {
namespace {

constexpr std::size_t kNoCutscene = std::numeric_limits<std::size_t>::max();

std::string lower(std::string_view text) {
    std::string out(text);
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

bool endsWithIgnoreCase(std::string_view text, std::string_view suffix) {
    if (suffix.size() > text.size()) return false;
    return lower(text.substr(text.size() - suffix.size())) == lower(suffix);
}

std::string_view pathBaseName(std::string_view path) {
    const std::size_t slash = path.find_last_of("\\/");
    if (slash != std::string_view::npos) path.remove_prefix(slash + 1);
    const std::size_t dot = path.find_last_of('.');
    if (dot != std::string_view::npos) path = path.substr(0, dot);
    return path;
}

bool pathMatchesSuffix(std::string_view path, std::string_view suffix) {
    return endsWithIgnoreCase(path, suffix) ||
           lower(pathBaseName(path)) == lower(suffix);
}

// Prefer an exact path component over a loose substring: "WilyMachine" must
// select the machine object, not "WilyMachine-Pillar". MaverickBass still wins
// for id "Bass" through the starts-with component rule.
int objectNameMatchScore(std::string_view name, std::string_view id) {
    if (id.empty()) return 0;
    const std::string wanted = lower(id);
    const std::string full = lower(name);
    const std::string base = lower(pathBaseName(name));
    if (base == wanted) return 4;

    int best = 0;
    std::size_t begin = 0;
    while (begin <= full.size()) {
        const std::size_t end = full.find_first_of("\\/", begin);
        const std::string_view part(
            full.data() + begin,
            end == std::string::npos ? full.size() - begin : end - begin);
        if (part == wanted) best = std::max(best, 3);
        else if (part.size() > wanted.size() &&
                 part.substr(0, wanted.size()) == wanted &&
                 (part[wanted.size()] == '-' || part[wanted.size()] == '_'))
            best = std::max(best, 2);
        if (end == std::string::npos) break;
        begin = end + 1;
    }
    if (best != 0) return best;
    return full.find(wanted) != std::string::npos ? 1 : 0;
}

} // namespace

CutsceneCommandKind cutsceneCommandKind(std::string_view name) {
    if (name == "DisplayObject") return CutsceneCommandKind::DisplayObject;
    if (name == "GetInGameObject") return CutsceneCommandKind::GetInGameObject;
    if (name == "MoveObject") return CutsceneCommandKind::MoveObject;
    if (name == "PlayAnim") return CutsceneCommandKind::PlayAnim;
    if (name == "KillObject") return CutsceneCommandKind::KillObject;
    if (name == "PlaySound") return CutsceneCommandKind::PlaySound;
    if (name == "StopSound") return CutsceneCommandKind::StopSound;
    if (name == "PlayMusicLoop") return CutsceneCommandKind::PlayMusicLoop;
    if (name == "PlayMusicOnce") return CutsceneCommandKind::PlayMusicOnce;
    if (name == "WriteText") return CutsceneCommandKind::WriteText;
    if (name == "EraseText") return CutsceneCommandKind::EraseText;
    if (name == "EndScene") return CutsceneCommandKind::EndScene;
    return CutsceneCommandKind::Unknown;
}

const char* cutsceneCommandName(CutsceneCommandKind kind) {
    switch (kind) {
    case CutsceneCommandKind::DisplayObject: return "DisplayObject";
    case CutsceneCommandKind::GetInGameObject: return "GetInGameObject";
    case CutsceneCommandKind::MoveObject: return "MoveObject";
    case CutsceneCommandKind::PlayAnim: return "PlayAnim";
    case CutsceneCommandKind::KillObject: return "KillObject";
    case CutsceneCommandKind::PlaySound: return "PlaySound";
    case CutsceneCommandKind::StopSound: return "StopSound";
    case CutsceneCommandKind::PlayMusicLoop: return "PlayMusicLoop";
    case CutsceneCommandKind::PlayMusicOnce: return "PlayMusicOnce";
    case CutsceneCommandKind::WriteText: return "WriteText";
    case CutsceneCommandKind::EraseText: return "EraseText";
    case CutsceneCommandKind::EndScene: return "EndScene";
    case CutsceneCommandKind::Unknown: return "Unknown";
    }
    return "Unknown";
}

std::size_t cutsceneIndexForKey(const AssetDatabase& assets, u32 key) {
    const std::vector<u32>& keys = assets.cutsceneKeys();
    for (std::size_t i = 0; i < keys.size(); ++i)
        if (keys[i] == key) return i;
    return kNoCutscene;
}

std::string_view cutscenePathForIndex(const AssetDatabase& assets, std::size_t index) {
    if (index < assets.cutscenePaths().size() && !assets.cutscenePaths()[index].empty())
        return assets.cutscenePaths()[index];
    if (index >= assets.cutsceneKeys().size()) return {};
    return cutscenePathForKey(assets.cutsceneKeys()[index]);
}

std::size_t findCutsceneByPathSuffix(const AssetDatabase& assets,
                                     std::string_view suffix) {
    if (suffix.empty()) return kNoCutscene;
    const std::size_t count = std::max(assets.cutscenes().size(),
                                       assets.cutsceneKeys().size());
    for (std::size_t i = 0; i < count; ++i)
        if (pathMatchesSuffix(cutscenePathForIndex(assets, i), suffix)) return i;
    return kNoCutscene;
}

std::string_view cutsceneSceneFolder(std::string_view scenePath) {
    const std::size_t base = scenePath.find_last_of("\\/");
    if (base == std::string_view::npos) return {};
    const std::string_view withoutBase = scenePath.substr(0, base);
    const std::size_t parent = withoutBase.find_last_of("\\/");
    if (parent == std::string_view::npos) return {};
    return withoutBase.substr(parent + 1);
}

namespace {

CutsceneSpriteRef spriteRefFromObject(const PackedArchive& pak,
                                      const PackedGameObject& object,
                                      std::string_view anim) {
    CutsceneSpriteRef ref;
    ref.gameObjectKey = object.key;
    ref.gameObjectName = object.name;

    const std::size_t begin = object.firstSprite;
    const std::size_t end = std::min<std::size_t>(
        pak.gameObjectSprites().size(),
        static_cast<std::size_t>(object.firstSprite) + object.spriteCount);
    if (begin >= end) return ref;

    std::size_t chosen = begin;
    if (!anim.empty()) {
        const u32 animKey = stateNameHash(anim);
        for (std::size_t i = begin; i < end; ++i) {
            if (pak.gameObjectSprites()[i].animKey == animKey) {
                chosen = i;
                break;
            }
        }
    }

    const PackedGameObjectSprite& link = pak.gameObjectSprites()[chosen];
    ref.spriteKey = link.spriteKey;
    if (link.spriteIndex >= pak.sprites().size()) return ref;
    const PackedSprite& sprite = pak.sprites()[link.spriteIndex];
    if (sprite.frameCount == 0 || sprite.frames == nullptr) return ref;
    const u32 imageIndex = sprite.frames[0].imageIndex;
    if (imageIndex >= pak.entries().size()) return ref;
    ref.imageIndex = imageIndex;
    ref.imageName = pak.entries()[imageIndex].name;
    return ref;
}

} // namespace

CutsceneSpriteRef resolveCutsceneSprite(const PackedArchive& pak,
                                        std::string_view scenePath,
                                        std::string_view resource,
                                        std::string_view anim,
                                        std::string_view id) {
    CutsceneSpriteRef ref;
    if (!pak.valid()) return ref;

    if (!resource.empty()) {
        std::string path = "..\\Cutscenes\\";
        path += cutsceneSceneFolder(scenePath);
        path += "\\";
        path += resource;
        const PackedGameObject* object = pak.findGameObject(stateNameHash(path));
        if (object != nullptr) return spriteRefFromObject(pak, *object, anim);
    }

    // GetInGameObject and PlayAnim name an already-created element instead of
    // a resource path. guns.mmu has no editor-side object name field, so match
    // the path of its first sprite (kept in PackedGameObject::name).
    if (!id.empty()) {
        const PackedGameObject* bestObject = nullptr;
        int bestScore = 0;
        for (const PackedGameObject& object : pak.gameObjects()) {
            const int score = objectNameMatchScore(object.name, id);
            if (score <= bestScore) continue;
            bestScore = score;
            bestObject = &object;
            if (score == 4) break;
        }
        if (bestObject != nullptr)
            return spriteRefFromObject(pak, *bestObject, anim);
    }

    // Last resort for PlayAnim without a resolvable resource: animation names
    // are unique enough in practice to identify the intended sprite.
    if (!anim.empty()) {
        const u32 animKey = stateNameHash(anim);
        for (const PackedGameObjectSprite& link : pak.gameObjectSprites()) {
            if (link.animKey != animKey || link.spriteIndex >= pak.sprites().size())
                continue;
            const PackedSprite& sprite = pak.sprites()[link.spriteIndex];
            if (sprite.frameCount == 0 || sprite.frames == nullptr) continue;
            const u32 imageIndex = sprite.frames[0].imageIndex;
            if (imageIndex >= pak.entries().size()) continue;
            ref.spriteKey = link.spriteKey;
            ref.imageIndex = imageIndex;
            ref.imageName = pak.entries()[imageIndex].name;
            return ref;
        }
    }
    return ref;
}

CutsceneElement* CutsceneRuntime::findElement(std::string_view id) {
    if (id.empty()) return nullptr;
    for (CutsceneElement& element : elements_)
        if (element.id == id) return &element;
    return nullptr;
}

void CutsceneRuntime::removeElement(std::string_view id) {
    if (id.empty()) return;
    const auto it = std::remove_if(elements_.begin(), elements_.end(),
                                   [id](const CutsceneElement& element) {
                                       return element.id == id;
                                   });
    elements_.erase(it, elements_.end());
}

void CutsceneRuntime::emit(CutsceneEvent event) {
    if (eventCount_ < events_.size()) {
        events_[(eventHead_ + eventCount_) % events_.size()] = std::move(event);
        ++eventCount_;
        return;
    }
    // Keep the newest commands when a debugging client falls behind.
    events_[eventHead_] = std::move(event);
    eventHead_ = (eventHead_ + 1) % events_.size();
}

CutsceneEvent CutsceneRuntime::pollEvent() {
    if (eventCount_ == 0) return {};
    CutsceneEvent event = std::move(events_[eventHead_]);
    eventHead_ = (eventHead_ + 1) % events_.size();
    --eventCount_;
    return event;
}

void CutsceneRuntime::apply(const CommandAsset& command, u32 timestampMs) {
    CutsceneEvent event;
    event.kind = cutsceneCommandKind(command.name);
    event.name = command.name;
    event.id = command.id;
    event.resource = command.resource;
    event.value = command.value;
    event.anim = command.anim;
    event.object = command.object;
    event.x = command.x;
    event.y = command.y;
    event.z = command.z;
    event.playThenKill = command.playThenKill;
    event.faceRight = command.faceRight;
    event.moveDuration = command.moveDuration;
    event.moveSpeed = command.moveSpeed;
    event.delayBetweenLetters = command.delayBetweenLetters;
    event.timestampMs = timestampMs;

    switch (event.kind) {
    case CutsceneCommandKind::DisplayObject:
    case CutsceneCommandKind::GetInGameObject: {
        removeElement(command.id);
        CutsceneElement element;
        element.id = command.id;
        element.resource = command.resource;
        element.x = static_cast<float>(command.x);
        element.y = static_cast<float>(command.y);
        element.z = static_cast<float>(command.z);
        element.fadeDelayMs = static_cast<float>(command.fadeDelay);
        element.targetX = element.x;
        element.targetY = element.y;
        elements_.push_back(std::move(element));
        break;
    }
    case CutsceneCommandKind::MoveObject: {
        if (CutsceneElement* element = findElement(command.id)) {
            element->targetX = static_cast<float>(command.x);
            element->targetY = static_cast<float>(command.y);
            element->moveSpeed = static_cast<float>(command.moveSpeed);
            if (command.moveDuration > 0) {
                const float dx = element->targetX - element->x;
                const float dy = element->targetY - element->y;
                const float distance = std::sqrt(dx * dx + dy * dy);
                element->moveSpeed = distance * 1000.0f /
                                     static_cast<float>(command.moveDuration);
            }
            element->moving = element->moveSpeed > 0.0f;
        }
        break;
    }
    case CutsceneCommandKind::PlayAnim: {
        if (CutsceneElement* element = findElement(command.id)) {
            element->anim = command.anim;
            element->playThenKill = command.playThenKill;
            element->faceRight = command.faceRight;
            if (!command.object.empty()) element->resource = command.object;
        }
        break;
    }
    case CutsceneCommandKind::KillObject:
        removeElement(command.id);
        break;
    case CutsceneCommandKind::WriteText: {
        CutsceneText text;
        text.id = command.id;
        text.value = command.value;
        text.x = static_cast<float>(command.x);
        text.y = static_cast<float>(command.y);
        text.z = static_cast<float>(command.z);
        text.delayBetweenLetters = command.delayBetweenLetters;
        const auto it = std::find_if(texts_.begin(), texts_.end(),
                                     [&](const CutsceneText& old) {
                                         return old.id == text.id;
                                     });
        if (it != texts_.end()) *it = std::move(text);
        else texts_.push_back(std::move(text));
        break;
    }
    case CutsceneCommandKind::EndScene:
        playing_ = false;
        break;
    case CutsceneCommandKind::EraseText:
        // The legacy dispatcher compares the name but no element effect was
        // observed in MMU.exe; preserve the event without inventing state.
        break;
    default:
        break;
    }

    emit(std::move(event));
}

bool CutsceneRuntime::startIndex(std::size_t index) {
    if (assets_ == nullptr || index >= assets_->cutscenes().size()) return false;
    stop();
    scene_ = &assets_->cutscenes()[index];
    sceneName_ = cutscenePathForIndex(*assets_, index);
    clockMs_ = 0.0f;
    nextTimestamp_ = 0;
    playing_ = true;
    return true;
}

bool CutsceneRuntime::startKey(u32 key) {
    if (assets_ == nullptr) return false;
    return startIndex(cutsceneIndexForKey(*assets_, key));
}

bool CutsceneRuntime::startPathSuffix(std::string_view suffix) {
    if (assets_ == nullptr) return false;
    return startIndex(findCutsceneByPathSuffix(*assets_, suffix));
}

void CutsceneRuntime::stop() {
    playing_ = false;
    scene_ = nullptr;
    sceneName_.clear();
    clockMs_ = 0.0f;
    nextTimestamp_ = 0;
    elements_.clear();
    texts_.clear();
    eventCount_ = 0;
    eventHead_ = 0;
}

void CutsceneRuntime::update(float dtSeconds) {
    if (!playing_ || scene_ == nullptr || dtSeconds <= 0.0f) return;
    clockMs_ += dtSeconds * 1000.0f;

    while (playing_ && nextTimestamp_ < scene_->timestamps.size() &&
           scene_->timestamps[nextTimestamp_].time <= clockMs_) {
        const TimeStampAsset& stamp = scene_->timestamps[nextTimestamp_];
        for (const CommandAsset& command : stamp.commands) {
            apply(command, stamp.time);
            if (!playing_) break;
        }
        ++nextTimestamp_;
    }

    if (!playing_) return;
    const float dt = dtSeconds;
    for (CutsceneElement& element : elements_) {
        if (!element.moving) continue;
        const float dx = element.targetX - element.x;
        const float dy = element.targetY - element.y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        const float step = element.moveSpeed * dt;
        if (distance <= step || distance <= 0.0001f) {
            element.x = element.targetX;
            element.y = element.targetY;
            element.moving = false;
            continue;
        }
        element.x += dx / distance * step;
        element.y += dy / distance * step;
    }
}

} // namespace mmu
