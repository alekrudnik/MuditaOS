#include "TransformFactory.hpp"

#include <Audio/AudioFormat.hpp>

#include "BasicDecimator.hpp"
#include "BasicInterpolator.hpp"
#include "MonoToStereo.hpp"
#include "NullTransform.hpp"
#include "Transform.hpp"
#include "TransformComposite.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

using audio::transcode::NullTransform;
using audio::transcode::Transform;
using audio::transcode::TransformFactory;

auto TransformFactory::makeTransform(AudioFormat sourceFormat, AudioFormat sinkFormat) const
    -> std::unique_ptr<Transform>
{
    auto transforms = std::vector<std::unique_ptr<Transform>>{};

    if (sourceFormat == sinkFormat) {
        return std::make_unique<NullTransform>();
    }

    if (sourceFormat.getBitWidth() != sinkFormat.getBitWidth()) {
        throw std::runtime_error("Bitwidth conversion is not implemented");
    }

    if (sourceFormat.getSampleRate() != sinkFormat.getSampleRate()) {
        transforms.push_back(getSamplerateTransform(sourceFormat, sinkFormat));
    }

    if (sourceFormat.getChannels() != sinkFormat.getChannels()) {
        transforms.push_back(getChannelsTransform(sourceFormat, sinkFormat));
    }

    // create composite if more than one transform
    if (transforms.size() == 1) {
        return std::move(transforms[0]);
    }
    else {
        auto transformsListForComposite = std::vector<std::shared_ptr<Transform>>{};
        std::move(std::begin(transforms), std::end(transforms), std::back_inserter(transformsListForComposite));
        return std::make_unique<audio::transcode::TransformComposite>(transformsListForComposite);
    }
}

auto TransformFactory::getSamplerateTransform(AudioFormat sourceFormat, AudioFormat sinkFormat) const
    -> std::unique_ptr<Transform>
{
    auto sourceRate = sourceFormat.getSampleRate();
    auto sinkRate   = sinkFormat.getSampleRate();

    auto greater = std::max(sourceRate, sinkRate);
    auto lesser  = std::min(sourceRate, sinkRate);

    if (greater % lesser != 0) {
        throw std::invalid_argument("Sample rate conversion is not supported");
    }

    auto ratio = greater / lesser;
    if (ratio != 2) {
        throw std::invalid_argument("Sample rate conversion is not supported (ratio != 2)");
    }

    if (sourceFormat.getBitWidth() != 16) {
        throw std::invalid_argument("Sample rate conversion with bit width other than 16 is not supported");
    }

    if (sourceFormat.getChannels() != 1) {
        throw std::invalid_argument("Sample rate conversion supported with mono only");
    }

    if (sourceRate > sinkRate) {
        return std::make_unique<audio::transcode::BasicDecimator<uint16_t, 1, 2>>();
    }
    else {
        return std::make_unique<audio::transcode::BasicInterpolator<uint16_t, 1, 2>>();
    }
}

auto TransformFactory::getChannelsTransform(AudioFormat sourceFormat, AudioFormat sinkFormat) const
    -> std::unique_ptr<Transform>
{
    if (sourceFormat.getChannels() == 1 && sinkFormat.getChannels() == 2 && sourceFormat.getBitWidth() == 16) {
        auto transform = std::make_unique<audio::transcode::MonoToStereo>();
        return transform;
    }
    else {
        throw std::invalid_argument("Channels conversion is not supported");
    }
}
