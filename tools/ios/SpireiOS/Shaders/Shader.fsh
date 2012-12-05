//
//  Shader.fsh
//  SpireiOS
//
//  Created by James Hughes on 12/5/12.
//  Copyright (c) 2012 SCI Institute. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
