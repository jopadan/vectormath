#include <stdio.h>

#include "vectormath.h"

int main()
{
    vec2 v = vec2(1, 2);        // vec2_new(1, 2)
    vec2 v1 = vec2(1);          // vec2_new1(1)

    printf("v = { %f, %f }\n", v.x, v.y);
    printf("v1 = { %f, %f }\n", v1.x, v1.y);


    vec3 v3 = vec3(3, 4, 5);    // vec3_new(3, 4, 5)
    vec3 v3_1 = vec3(3);        // vec3_new1(3)

    printf("v3 = { %f, %f, %f }\n", v3.x, v3.y, v3.z);
    printf("v3_1 = { %f, %f, %f }\n", v3_1.x, v3_1.y, v3_1.z);


    vec4 v4 = vec4(6, 7, 8, 9); // vec4_new(6, 7, 8, 9)
    vec4 v4_1 = vec4(4);        // vec4_new1(4)

    printf("v4 = { %f, %f, %f, %f }\n", v4.x, v4.y, v4.z, v4.w);
    printf("v4_1 = { %f, %f, %f, %f }\n", v4_1.x, v4_1.y, v4_1.z, v4_1.w);


    vec2 v2_v3 = vec2(v3);      // vec2_from_vec3(v3)
    vec2 v2_v4 = vec2(v4);      // vec2_from_vec4(v4)
    vec3 v3_v2 = vec3(v, 0.0f);       // vec3_from_vec2(v)
    vec3 v3_v4 = vec3(v4);      // vec3_from_vec4(v4)

    printf("v2_v3 = { %f, %f }\n", v2_v3.x, v2_v3.y);
    printf("v2_v4 = { %f, %f }\n", v2_v4.x, v2_v4.y);
    printf("v3_v2 = { %f, %f, %f }\n", v3_v2.x, v3_v2.y, v3_v2.z);
    printf("v3_v4 = { %f, %f, %f }\n", v3_v4.x, v3_v4.y, v3_v4.z);

    
    vec2 v2_v2 = vec2(v);       // vec2_copy(v) == v
    vec3 v3_v3 = vec3(v3);      // vec3_copy(v3) == v3
    vec4 v4_v4 = vec4(v4);      // vec4_copy(v4) == v4

    printf("v2_v2 = { %f, %f }\n", v2_v2.x, v2_v2.y);
    printf("v3_v3 = { %f, %f, %f }\n", v3_v3.x, v3_v3.y, v3_v3.z);
    printf("v4_v4 = { %f, %f, %f }\n", v4_v4.x, v4_v4.y, v4_v4.z);

	return 0;
}

//! LEAVE AN EMPTY LINE HERE, REQUIRE BY GCC/G++
