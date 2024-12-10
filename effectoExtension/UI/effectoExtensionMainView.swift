//
//  effectoExtensionMainView.swift
//  effectoExtension
//
//  Created by Andreas Mueller on 10/12/2024.
//

import SwiftUI

struct effectoExtensionMainView: View {
    var parameterTree: ObservableAUParameterGroup
    
    var body: some View {
        ParameterSlider(param: parameterTree.global.gain)
    }
}
