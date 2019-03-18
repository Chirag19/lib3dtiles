/**
 * Copyright (c) 2019 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef threedtiles_3dtiles_hpp_included_
#define threedtiles_3dtiles_hpp_included_

#include <iosfwd>

#include <vector>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "utility/enum-io.hpp"
#include "utility/openmp.hpp"

#include "math/geometry_core.hpp"

namespace threedtiles {

using OptString = boost::optional<std::string>;
using ExtensionList = std::vector<std::string>;
using Extension = boost::any;
using Extensions = std::map<std::string, Extension>;

UTILITY_GENERATE_ENUM(Refinement,
                      ((replace)("REPLACE"))
                      ((add)("ADD"))
                      )

struct CommonBase {
    Extensions extensions;
    boost::any extras;
};

struct Box : CommonBase {
    math::Point3 center;
    math::Point3 x;
    math::Point3 y;
    math::Point3 z;

    void update(const Box &other);
};

struct Region : CommonBase {
    math::Extents3 extents;

    Region() : extents(math::InvalidExtents{}) {}

    void update(const Region &other);
};

struct Sphere : CommonBase {
    math::Point3 center;
    double radius;

    void update(const Sphere &other);
};

using BoundingVolume = boost::variant<Box, Region, Sphere>;

struct Property : CommonBase {
    double minimum;
    double maximum;

    Property() : minimum(), maximum() {}
};

using Properties = std::map<std::string, Property>;

struct TileContent : CommonBase {
    boost::optional<BoundingVolume> boundingVolume;
    std::string uri;
};

struct Tile : CommonBase {
    typedef std::shared_ptr<Tile> pointer;
    typedef std::vector<pointer> list;

    BoundingVolume boundingVolume;
    boost::optional<BoundingVolume> viewerRequestVolume;
    double geometricError;
    boost::optional<Refinement> refine;
    boost::optional<math::Matrix4> transform; // serialize as column major!
    boost::optional<TileContent> content;
    Tile::list children;
};

struct Asset : CommonBase {
    std::string version;
    OptString tilesetVersion;

    Asset() : version("1.0") {}
};

struct Tileset : CommonBase {
    Asset asset;
    Properties properties;
    double geometricError;
    Tile::pointer root;

    ExtensionList extensionsUsed;
    ExtensionList extensionsRequired;
};

/** Write a tileset JSON file to an output stream.
 */
void write(std::ostream &os, const Tileset &tileset);

/** Write a tileset JSON file to an output file.
 */
void write(const boost::filesystem::path &path, const Tileset &tileset);

/** Updates one volume from the other.
 *  If updated is valid then it has to be of the same type as the updater.
 */
void update(boost::optional<BoundingVolume> &updated
            , const boost::optional<BoundingVolume> &updater);

/** Read tileset JSON file from an output stream.
 *
 *  Path is used to resolve relative paths to absolute paths.
 */
void read(std::istream &is, Tileset &tileset
          , const boost::filesystem::path &path = "");

} // namespace threedtiles

#endif // 3dtiles_3dtiles_hpp_included_
