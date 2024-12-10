//
//  effectoApp.swift
//  effecto
//
//  Created by Andreas Mueller on 10/12/2024.
//

import SwiftUI

@main
struct effectoApp: App {
    @State private var hostModel = AudioUnitHostModel()

    var body: some Scene {
        WindowGroup {
            ContentView(hostModel: hostModel)
        }
    }
}
