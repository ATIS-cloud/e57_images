#include <fstream>
#include <vector>
#include <limits>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <iostream>

#include "E57Foundation.h"
#include "E57Simple.h"
#include "LASReader.h"

using namespace e57;
using namespace std;

struct Image
{
    string name;
    string guid;
    string path;
    int width;
    int height;
    float pixelWidth;
    float pixelHeight;
    double posX;
    double posY;
    double posZ;
    double rotX;
    double rotY;
    double rotZ;
    double rotW;
};

vector<uint8_t> readImage(const string imagePath, size_t& imageSize)
{
    ifstream file(imagePath, ios::binary);
    file.seekg(0, ios::end);
    streamsize size = file.tellg();
    cout << size << endl;
    file.seekg(0, ios::beg);
    vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();
    imageSize = size;
    return buffer;
}

void insertImage(VectorNode& images2D, ImageFile& imf, const Image image)
{
    StructureNode picture = StructureNode(imf);
    images2D.append(picture);

    /// Attach a guid to the picture.
    picture.set("guid", StringNode(imf, image.guid));
    picture.set("name", StringNode(imf, image.name));
    picture.set("sensorVendor", StringNode(imf, "You scanner / software name"));

    // Make an area holding spherical projection parameters for the picture.
    StructureNode spherical = StructureNode(imf);
    picture.set("sphericalRepresentation", spherical);

    /// Add spherical parameters
    spherical.set("imageWidth",      IntegerNode(imf, image.width));
    spherical.set("imageHeight",     IntegerNode(imf, image.height));
    spherical.set("pixelWidth",      FloatNode(imf, image.pixelWidth));
    spherical.set("pixelHeight"  ,   FloatNode(imf, image.pixelHeight));

    /// Save image data to the E57 file
    size_t imageSize = -1;
    vector<uint8_t> imageContent = readImage(image.path, imageSize);
    BlobNode jpegImage = BlobNode(imf, imageSize);
    spherical.set("jpegImage", jpegImage);
    jpegImage.write(imageContent.data(), 0, imageSize);

    /// Attach a pose to picture (position and a rotation quaternion)
    StructureNode picturePose = StructureNode(imf);
    picture.set("pose", picturePose);
    StructureNode pictureRotation = StructureNode(imf);
    picturePose.set("rotation", pictureRotation);
    pictureRotation.set("w", FloatNode(imf, image.rotW));
    pictureRotation.set("x", FloatNode(imf, image.rotX));
    pictureRotation.set("y", FloatNode(imf, image.rotY));
    pictureRotation.set("z", FloatNode(imf, image.rotZ));
    StructureNode pictureTranslation = StructureNode(imf);
    picturePose.set("translation", pictureTranslation);
    pictureTranslation.set("x", FloatNode(imf, image.posX));
    pictureTranslation.set("y", FloatNode(imf, image.posY));
    pictureTranslation.set("z", FloatNode(imf, image.posZ));
}

int main(int /*argc*/, char** /*argv*/)
{
    try {
        /// Open new file for writing, get the initialized root node (a Structure).
        /// Path name: "/"
        ImageFile imf("foo.e57", "w");
        StructureNode root = imf.root();

        // Open the JSON file
        std::ifstream file("/app/src/examples/image.json");

        if (!file.is_open()) {
            std::cerr << "Error: Could not open the file." << std::endl;
            return 1;
        }

        // Parse the JSON file
        nlohmann::json json_data;
        try {
            file >> json_data;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            return 1;
        }

        vector<Image> images = vector<Image>();
        for (const auto& element : json_data) {
            Image image;
            image.name = element["name"];
            image.guid = element["guid"];
            image.path = element["path"];
            image.width = element["width"];
            image.height = element["height"];
            image.pixelWidth = element["pixelWidth"];
            image.pixelHeight = element["pixelHeight"];
            image.posX = element["pos_x"];
            image.posY = element["pos_y"];
            image.posZ = element["pos_z"];
            image.rotX = element["rot_x"];
            image.rotY = element["rot_y"];
            image.rotZ = element["rot_z"];
            image.rotW = element["rot_w"];
            images.push_back(image);
        }

        // Read unstructured E57 Data
        const string e57Path = "/app/src/examples/atis_rcn.e57";
        e57::Reader eReader(e57Path);

        int64_t nColumn = 0;
        int64_t nRow = 0;
        int64_t nPointsSize = 0;
        int64_t nGroupsSize = 0;
        int64_t nCountsSize = 0;
        bool bColumnIndex = 0;
        int scanIndex = 0;

        e57::Data3D	scanHeader;
	    eReader.ReadData3D( scanIndex, scanHeader);
	    e57::RigidBodyTransform scanTransformation = scanHeader.pose;
	    e57::Translation scanTranslation = scanTransformation.translation;

        eReader.GetData3DSizes( scanIndex, nRow, nColumn, nPointsSize, nGroupsSize, nCountsSize, bColumnIndex);

        int64_t nSize = (nRow > 0) ? nRow : 1024;	//Pick a size for buffers

        double *xData = new double[nPointsSize];
        double *yData = new double[nPointsSize];
        double *zData = new double[nPointsSize];
        int8_t *cartesianInvalidState = new int8_t[nPointsSize];
        double *intensity = new double[nPointsSize];
        int8_t *intensityInvalidState = new int8_t[nPointsSize];
        uint16_t *colorRed = new uint16_t[nPointsSize];
        uint16_t *colorGreen = new uint16_t[nPointsSize];
        uint16_t *colorBlue = new uint16_t[nPointsSize];

        e57::CompressedVectorReader dataReader = eReader.SetUpData3DPointsData(
                                            scanIndex,nPointsSize,xData,yData,zData,
                                            cartesianInvalidState, intensity, intensityInvalidState,
                                            colorRed, colorGreen, colorBlue
                                            );

        dataReader.read();
        dataReader.close();

        for (int i = 0; i < nPointsSize; i++)
        {
            xData[i] += scanTranslation.x;
            yData[i] += scanTranslation.y;
            zData[i] += scanTranslation.z;
        }

        // Calculate bounding box
        double xMax = numeric_limits<double>::min();
        double xMin = numeric_limits<double>::max();
        double yMax = numeric_limits<double>::min();
        double yMin = numeric_limits<double>::max();
        double zMax = numeric_limits<double>::min();
        double zMin = numeric_limits<double>::max();

        for (int i = 0; i < nPointsSize; i++)
        {
            double x = xData[i];
            double y = yData[i];
            double z = zData[i];

            xMax = max(x, xMax);
            xMin = min(x, xMin);
            yMax = max(y, yMax);
            yMin = min(y, yMin);
            zMax = max(z, zMax);
            zMin = min(z, zMin);
        }

        double centerX = (xMax + xMin) / 2;
        double centerY = (yMax + yMin) / 2;
        double centerZ = (zMax + zMin) / 2;

        /// Set per-file properties.
        /// Path names: "/formatName", "/majorVersion", "/minorVersion", "/coordinateMetadata"
        root.set("formatName", StringNode(imf, ""));
        root.set("guid", StringNode(imf, "3f2504e0-4f89-11d3-9a0c-0305e82c3300"));

        /// Get ASTM version number supported by library, so can write it into file
        int astmMajor;
        int astmMinor;
        ustring libraryId;
        E57Utilities().getVersions(astmMajor, astmMinor, libraryId);
        root.set("versionMajor", IntegerNode(imf, astmMajor));
        root.set("versionMinor", IntegerNode(imf, astmMinor));

        /// Create creationDateTime structure
        /// Path name: "/creationDateTime
        StructureNode creationDateTime = StructureNode(imf);
        root.set("creationDateTime", creationDateTime);
        creationDateTime.set("dateTimeValue", FloatNode(imf, 123.456)); //!!! convert time() to GPStime

        /// Create 3D data area.
        /// Path name: "/data3D"
        VectorNode data3D = VectorNode(imf, true);
        root.set("data3D", data3D);

        /// Add first scan
        /// Path name: "/data3D/0"
        StructureNode scan0 = StructureNode(imf);
        data3D.append(scan0);

        /// Add guid to scan0.
        /// Path name: "/data3D/0/guid".
        const char* scanGuid0 = "{796D1E93-7A5F-4657-A002-7060934B3D65}";
        scan0.set("guid", StringNode(imf, scanGuid0));

        /// Make a prototype of datatypes that will be stored in points record.
        /// This prototype will be used in creating the points CompressedVector.
        /// Using this proto in a CompressedVector will define path names like:
        ///      "/data3D/0/points/0/cartesianX"
        StructureNode proto = StructureNode(imf);
        proto.set("cartesianX",  FloatNode(imf, centerX, E57_SINGLE, xMin, xMax));
        proto.set("cartesianY",  FloatNode(imf, centerY, E57_SINGLE, yMin, yMax));
        proto.set("cartesianZ",  FloatNode(imf, centerZ, E57_SINGLE, zMin, zMax));
        proto.set("colorRed",    FloatNode(imf, 0.0, E57_SINGLE, 0.0, 1.0));
        proto.set("colorGreen",  FloatNode(imf, 0.0, E57_SINGLE, 0.0, 1.0));
        proto.set("colorBlue",   FloatNode(imf, 0.0, E57_SINGLE, 0.0, 1.0));

        /// Make empty codecs vector for use in creating points CompressedVector.
        /// If this vector is empty, it is assumed that all fields will use the BitPack codec.
        VectorNode codecs = VectorNode(imf, true);

        /// Create CompressedVector for storing points.  Path Name: "/data3D/0/points".
        /// We use the prototype and empty codecs tree from above.
        /// The CompressedVector will be filled by code below.
        CompressedVectorNode points = CompressedVectorNode(imf, proto, codecs);
        scan0.set("points", points);

        /// Create pose structure for scan.
        /// Path names: "/data3D/0/pose/rotation/w", etc...
        ///             "/data3D/0/pose/translation/x", etc...
        StructureNode pose = StructureNode(imf);
        scan0.set("pose", pose);
        StructureNode rotation = StructureNode(imf);
        pose.set("rotation", rotation);
        rotation.set("w", FloatNode(imf, 1.0));
        rotation.set("x", FloatNode(imf, 0.0));
        rotation.set("y", FloatNode(imf, 0.0));
        rotation.set("z", FloatNode(imf, 0.0));
        StructureNode translation = StructureNode(imf);
        pose.set("translation", translation);
        translation.set("x", FloatNode(imf, 0.0));
        translation.set("y", FloatNode(imf, 0.0));
        translation.set("z", FloatNode(imf, 0.0));

        ///================
        /// Add name and description to scan
        /// Path names: "/data3D/0/name", "/data3D/0/description".
        scan0.set("name", StringNode(imf, "test_scan"));

        ///================
        /// Create 2D image area.
        /// Path name: "/images2D"
        VectorNode images2D = VectorNode(imf, true);
        root.set("images2D", images2D);

        for (const auto& single_image : images)
            insertImage(images2D, imf, single_image);

        ///================
        /// Prepare vector of source buffers for writing in the CompressedVector of points
        vector<SourceDestBuffer> sourceBuffers;
        sourceBuffers.push_back(SourceDestBuffer(imf, "cartesianX",  xData,  nPointsSize, true, true));
        sourceBuffers.push_back(SourceDestBuffer(imf, "cartesianY",  yData,  nPointsSize, true, true));
        sourceBuffers.push_back(SourceDestBuffer(imf, "cartesianZ",  zData,  nPointsSize, true, true));
        sourceBuffers.push_back(SourceDestBuffer(imf, "colorRed",    colorRed,    nPointsSize, true));
        sourceBuffers.push_back(SourceDestBuffer(imf, "colorGreen",  colorGreen,  nPointsSize, true));
        sourceBuffers.push_back(SourceDestBuffer(imf, "colorBlue",   colorBlue,   nPointsSize, true));

        /// Write source buffers into CompressedVector
        {
            CompressedVectorWriter writer = points.writer(sourceBuffers);
            writer.write(nPointsSize);
            writer.close();
        }

        // Free used memory
        delete xData;
        delete yData;
        delete zData;
        delete cartesianInvalidState;
        delete intensity;
        delete intensityInvalidState;
        delete colorRed;
        delete colorGreen;
        delete colorBlue;

        imf.close();
    } catch(E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__);
    } catch (std::exception& ex) {
        cerr << "Got an std::exception, what=" << ex.what() << endl;
    } catch (...) {
        cerr << "Got an unknown exception" << endl;
    }
    return(0);
}
