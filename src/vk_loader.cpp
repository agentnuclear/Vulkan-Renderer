﻿
#include <vk_loader.h>
#include "stb_image.h"
#include <iostream>
#include <vk_loader.h>

#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_types.h"
#include <glm/gtx/quaternion.hpp>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>


std::optional<AllocatedImage> load_image(VulkanEngine* engine, fastgltf::Asset& asset, fastgltf::Image& image)
{
    AllocatedImage newImage{};

    int width, height, nrCHannel;

    std::visit(
        fastgltf::visitor{
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filepath) {
            assert(filepath.fileByteOffset == 0);
            assert(filepath.uri.isLocalPath());

            const std::string path(filepath.uri.path().begin(), filepath.uri.path().end());
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrCHannel, 4);
            if (data) {
                VkExtent3D imagesize;
                imagesize.width = width;
                imagesize.height = height;
                imagesize.depth = 1;

                newImage = engine->create_image(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);

                stbi_image_free(data);
                }
            },
        [&](fastgltf::sources::Vector& vector) {
            unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()), &width, &height, &nrCHannel,4);
            if (data) {
                VkExtent3D imagesize;
                imagesize.width = width;
                imagesize.height = height;
                imagesize.depth = 1;

                newImage = engine->create_image(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);

                stbi_image_free(data);
            }
        },
        
        [&](fastgltf::sources::BufferView& view) {
            auto& bufferView = asset.bufferViews[view.bufferViewIndex];
            auto& buffer = asset.buffers[bufferView.bufferIndex];
            std::visit(fastgltf::visitor{
                [](auto& arg) {},
                [&](fastgltf::sources::Vector& vector) {
                    unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                       static_cast<int>(bufferView.byteLength),
                                       &width, &height, &nrCHannel, 4);
                    if (data) {
                        VkExtent3D imagesize;
                        imagesize.width = width;
                        imagesize.height = height;
                        imagesize.depth = 1;

                        newImage = engine->create_image(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);

                        stbi_image_free(data);
                    }
                }
                }, buffer.data);
        },

      }, image.data);

    // if any of the attempts to load the data failed , we havent written image so hangle is null
    if (newImage.image == VK_NULL_HANDLE) {
        return{};
   
    }
    else {
        return newImage;
    }
}

VkFilter extract_filter(fastgltf::Filter filter) {
    switch (filter) {
        //nearest sampler
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
        return VK_FILTER_NEAREST;

        //linear sampler
    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapLinear:
    case fastgltf::Filter::LinearMipMapNearest:
    default:
        return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode extract_mipmap_mode(fastgltf::Filter filter) {
    switch (filter) {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}


std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanEngine* engine, std::string_view filePath)
{

    fmt::print("Loading GLTF: {}", filePath);

    std::shared_ptr<LoadedGLTF> scene = std::make_shared<LoadedGLTF>();
    scene->creator = engine;
    LoadedGLTF& file = *scene.get();

    fastgltf::Parser parser{};

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

    fastgltf::GltfDataBuffer data;
    data.loadFromFile(filePath);

    fastgltf::Asset gltf;

    std::filesystem::path path = filePath;

    auto type = fastgltf::determineGltfFileType(&data);
    if (type == fastgltf::GltfType::glTF) {
        auto load = parser.loadGLTF(&data, path.parent_path(), gltfOptions);
        if (load) {
            gltf = std::move(load.get());
        }
        else {
            std::cerr << "Failed to load gltf: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else if (type == fastgltf::GltfType::GLB) {
        auto load = parser.loadBinaryGLTF(&data, path.parent_path(), gltfOptions);
        if (load) {
            gltf = std::move(load.get());
        }
        else {
            std::cerr << "Failed to load gltf: " << fastgltf::to_underlying(load.error()) << std::endl;
            return{};
        }
    }
    else {
        std::cerr << "Failed to determine glTF container" << std::endl;
        return{};
    }

    // we can estimate the descriptors we will need accurately
    std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> sizes = { {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3},{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1} };

    file.descriptorPool.init(engine->_device, gltf.materials.size(), sizes);

    //temporal arrays for all the objs to use while createing the gltf data
    std::vector<std::shared_ptr<MeshAsset>> meshes;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<AllocatedImage> images;
    std::vector<std::shared_ptr<GLTFMaterial>> materials;

    //load samplers
    for (fastgltf::Sampler& sampler : gltf.samplers) {
        VkSamplerCreateInfo sampl = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
        sampl.maxLod = VK_LOD_CLAMP_NONE;
        sampl.minLod = 0;

        sampl.magFilter = extract_filter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
        sampl.minFilter = extract_filter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        sampl.mipmapMode = extract_mipmap_mode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        VkSampler newSampler;
        VkResult result = vkCreateSampler(engine->_device, &sampl, nullptr, &newSampler);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to create sampler" << result << std::endl;
            return {};
        }

        file.samplers.push_back(newSampler);

        // load all textures
        for (fastgltf::Image& image : gltf.images) {
            std::optional<AllocatedImage> img = load_image(engine, gltf, image);

            if (img.has_value()) {
                images.push_back(*img);
                file.images[image.name.c_str()] = *img;
            }
            else {
                // failed to load the the texture so sending the error texture instead of breaking the loading
                images.push_back(engine->_errorCheckerboardImage);
                std::cout << "gltf failed to load textures" << image.name << std::endl;
            }
        }

        //create buffer to hold the material data
        file.materialDataBuffer = engine->create_buffer(sizeof(GLTFMetallic_Roughness::MaterialConstants) * gltf.materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        int data_index = 0;
        GLTFMetallic_Roughness::MaterialConstants* sceneMaterialCOnstants = (GLTFMetallic_Roughness::MaterialConstants*)file.materialDataBuffer.info.pMappedData;

        //loop to load materials
        for (fastgltf::Material& mat : gltf.materials) {
            std::shared_ptr<GLTFMaterial> newMat = std::make_shared<GLTFMaterial>();
            materials.push_back(newMat);
            file.materials[mat.name.c_str()] = newMat;

            GLTFMetallic_Roughness::MaterialConstants constants;
            constants.colorFactors.x = mat.pbrData.baseColorFactor[0];
            constants.colorFactors.y = mat.pbrData.baseColorFactor[1];
            constants.colorFactors.z = mat.pbrData.baseColorFactor[2];
            constants.colorFactors.w = mat.pbrData.baseColorFactor[3];

            constants.metal_rough_factors.x = mat.pbrData.metallicFactor;
            constants.metal_rough_factors.y = mat.pbrData.roughnessFactor;
            // write the material params to the buffer
            sceneMaterialCOnstants[data_index] = constants;

            MaterialPass passtype = MaterialPass::MainColor;
            if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
                passtype = MaterialPass::Transparent;
            }

            GLTFMetallic_Roughness::MaterialResources materialResources;
            //default the material textures
            materialResources.colorImage = engine->_whiteImage;
            materialResources.colorSampler = engine->_defaultSamplerLinear;
            materialResources.metalRoughImage = engine->_whiteImage;
            materialResources.metalRoughSampler = engine->_defaultSamplerLinear;

            //set the unitform buffer for the mateiral data
            materialResources.dataBuffer = file.materialDataBuffer.buffer;
            materialResources.dataBufferOffset = data_index * sizeof(GLTFMetallic_Roughness::MaterialConstants);

            //grab texture from the gltf file
            if (mat.pbrData.baseColorTexture.has_value()) {
                size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
                size_t sampler = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value();

                materialResources.colorImage = images[img];
                materialResources.colorSampler = file.samplers[sampler];

            }
            newMat->data = engine->metalRoughMaterial.write_material(engine->_device, passtype, materialResources, file.descriptorPool);
            data_index++;
        }

        // use the same vectors for all meshes so that the memory dosent relocate as often
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;

        for (fastgltf::Mesh& mesh : gltf.meshes) {
            std::shared_ptr<MeshAsset> newmesh = std::make_shared<MeshAsset>();
            meshes.push_back(newmesh);
            file.meshes[mesh.name.c_str()] = newmesh;
            newmesh->name = mesh.name;

            //clear the mesh arrays each mesh , we dont want to merge them by error
            indices.clear();
            vertices.clear();

            for (auto&& p : mesh.primitives) {
                GeoSurface newsurface;
                newsurface.startIndex = (uint32_t)indices.size();
                newsurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

                size_t initial_vtx = vertices.size();

                //load indices
                {
                    fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexaccessor.count);

                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor, [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vtx);
                        });
                }


                //load vertex positions
                {
                    fastgltf::Accessor& posAccessors = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessors.count);

                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessors, [&](glm::vec3 v, size_t index) {
                        Vertex newvtx;
                        newvtx.position = v;
                        newvtx.normal = { 1,0,0 };
                        newvtx.color = glm::vec4{ 1.f };
                        newvtx.uv_x = 0;
                        newvtx.uv_y = 0;
                        vertices[initial_vtx + index] = newvtx;
                        });

                }

                //load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second], [&](glm::vec3 v, size_t index) {
                        vertices[initial_vtx + index].normal = v;
                        });
                }

                //load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second], [&](glm::vec2 v, size_t index) {
                        vertices[initial_vtx + index].uv_x = v.x;
                        vertices[initial_vtx + index].uv_y = v.y;
                        });
                }

                //load vertex colors
                auto colors = p.findAttribute("COLOR_0");
                if (colors != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second], [&](glm::vec4 v, size_t index) {
                        vertices[initial_vtx + index].color = v;
                        });
                }

                if (p.materialIndex.has_value()) {
                    newsurface.material = materials[p.materialIndex.value()];
                }
                else {
                    newsurface.material = materials[0];
                }

                //loop the vertices of this surface, find min/max bounds
                glm::vec3 minpos = vertices[initial_vtx].position;
                glm::vec3 maxpos = vertices[initial_vtx].position;
                for (int i = initial_vtx; i < vertices.size(); i++) {
                    minpos = glm::min(minpos, vertices[i].position);
                    maxpos = glm::max(maxpos, vertices[i].position);
                }
                //calculate origin and extents from the min/max, use extent length for radius
                newsurface.bounds.origin = (maxpos + minpos) / 2.f;
                newsurface.bounds.extents = (maxpos - minpos) / 2.f;
                newsurface.bounds.sphereRadius = glm::length(newsurface.bounds.extents);

                newmesh->surfaces.push_back(newsurface);
            }
            newmesh->meshBuffers = engine->uploadMesh(indices, vertices);
        }

        // load all nodes and thier meshes
        for (fastgltf::Node& node : gltf.nodes) {
            std::shared_ptr<Node> newNode;

            //find if the node has a mesh and if it does hook it to the mesh pointer and allocate it with the meshnode class
            if (node.meshIndex.has_value()) {
                newNode = std::make_shared<MeshNode>();
                static_cast<MeshNode*>(newNode.get())->mesh = meshes[*node.meshIndex];
            }
            else {
                newNode = std::make_shared<Node>();
            }
            nodes.push_back(newNode);
            file.nodes[node.name.c_str()] = newNode;

            std::visit(fastgltf::visitor{ [&](fastgltf::Node::TransformMatrix matrix) {memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
                },
                [&](fastgltf::Node::TRS transform) {
                    glm::vec3 tl(transform.translation[0], transform.translation[1],transform.translation[2]);
                    glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);
                    glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                    glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
                    glm::mat4 rm = glm::toMat4(rot);
                    glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

                    newNode->localTransform = tm * rm * sm;
                        } },
                node.transform);
        }

        //run loop again to setup transformation hierarchy
        for (int i = 0; i < gltf.nodes.size(); i++) {
            fastgltf::Node& node = gltf.nodes[i];
            std::shared_ptr<Node>& sceneNode = nodes[i];

            for (auto& c : node.children) {
                sceneNode->children.push_back(nodes[c]);
                nodes[c]->parent = sceneNode;
            }
        }

        //find the top nodes , with no parents
        for (auto& node : nodes) {
            if (node->parent.lock() == nullptr) {
                file.topNodes.push_back(node);
                node->refreshTransform(glm::mat4{ 1.f });
            }
        }
    }
    return scene;
}


void LoadedGLTF::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{
    //create renderables from the scenenodes
    for (auto& n : topNodes) {
        n->Draw(topMatrix, ctx);
    }
}

void LoadedGLTF::clearAll()
{
    VkDevice dv = creator->_device;

    descriptorPool.destroy_pools(dv);
    creator->destroy_buffer(materialDataBuffer);

    for (auto& [k, v] : meshes) {
        creator->destroy_buffer(v->meshBuffers.indexBuffer);
        creator->destroy_buffer(v->meshBuffers.vertexBuffer);
    }

    for (auto& [k, v] : images) {
        if (v.image == creator->_errorCheckerboardImage.image) {
            continue;
        }
        creator->destroy_image(v);
    }
    for (auto& sampler : samplers) {
        vkDestroySampler(dv, sampler, nullptr);
    }
}
