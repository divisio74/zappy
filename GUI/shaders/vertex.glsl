#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aBoneIds;
layout(location = 4) in vec4 aWeights;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 boneMatrices[100];

void main()
{
    vec4 skinnedPos = vec4(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < 4; i++) {
        if (aWeights[i] == 0.0) continue;
        skinnedPos += aWeights[i] * boneMatrices[aBoneIds[i]] * vec4(aPos, 1.0);
        totalWeight += aWeights[i];
    }

    if (totalWeight == 0.0)
        skinnedPos = vec4(aPos, 1.0);

    gl_Position = projection * view * model * skinnedPos;
    TexCoord = aTexCoord;
}