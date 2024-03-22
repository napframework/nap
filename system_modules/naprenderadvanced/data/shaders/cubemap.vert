// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

out vec2 passUV;

void main()
{
	passUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);	
	gl_Position = vec4(passUV * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
}
