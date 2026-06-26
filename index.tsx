import React, {useState, useEffect} from "react"
import {createRoot} from "react-dom/client"
import DarkIcon from "./assets/dark.svg"
import LightIcon from "./assets/light.svg"
import ButtonSlider from "./components/ButtonSlider"
import Knob from "./components/Knob"
import Waveform from "./components/Waveform"
import LFOBar from "./components/LFOBar"
import PresetBar from "./components/PresetBar"
import parameters from "./processor/parameters.json"
import functions from "./structures/Functions"
import "./index.scss"

const darkColorList = {
    "--background": "#220E1A",
    "--textColor": "#FFFFFF"
}

const lightColorList = {
    "--background": "#FFBEE6",
    "--textColor": "#000000",
}

type ThemeContextType = {theme: string; setTheme: React.Dispatch<React.SetStateAction<string>>}
export const ThemeContext = React.createContext<ThemeContextType>({theme: "", setTheme: () => null})

const App: React.FunctionComponent = () => {
    const [theme, setTheme] = useState(localStorage.getItem("theme") || "light")

    useEffect(() => {
        window.__JUCE__.backend.emitEvent("themeChange", {theme})
    }, [])

    useEffect(() => {
        const colorList = theme === "light" ? lightColorList : darkColorList
        for (const [key, color] of Object.entries(colorList)) {
            document.documentElement.style.setProperty(key, color)
        }
        localStorage.setItem("theme", theme)
        window.__JUCE__.backend.emitEvent("themeChange", {theme})
    }, [theme])

    const toggleTheme = () => {
        setTheme((prev) => prev === "light" ? "dark" : "light")
    }

    const filter = functions.calculateFilter("#FF4EA7")

    return (
        <div className="app">
            <ThemeContext.Provider value={{theme, setTheme}}>
            <div className="title-container">
                <span className="title-text">Cute Crush</span>
                {theme === "light" ? 
                <DarkIcon className="theme-icon" style={{filter}} onClick={toggleTheme}/> :
                <LightIcon className="theme-icon" style={{filter}} onClick={toggleTheme}/>}
            </div>
            <div className="sample-rate-container">
                <ButtonSlider 
                    label={"SAMPLE RATE"} 
                    parameterID={parameters.sampleRate.id}/>
            </div>
             <div className="knobs-container">
                <Waveform
                    parameterID={parameters.sampleRate.id}/>
                <Knob 
                    label={"BITS"} 
                    parameterID={parameters.bitDepth.id} 
                    color="#FF4EA7" 
                    display="bits"/>
                <Knob 
                    label={parameters.mix.id.toUpperCase()} 
                    parameterID={parameters.mix.id} 
                    color="#FF4EA7" 
                    display="percent"/>
            </div>
            <div className="lfo-container">
                <LFOBar 
                    label={"CRUSH"}
                    lfoTypeID={parameters.crushLFOType.id} 
                    lfoRateID={parameters.crushLFORate.id} 
                    lfoAmountID={parameters.crushLFOAmount.id} 
                    lfoInvertID={parameters.crushLFOInvert.id}
                    color="#FF4EA7"
                    theme={theme}/>
            </div>
            <div className="preset-container">
                <PresetBar/>
            </div>
            </ThemeContext.Provider>
        </div>
    )

}

const root = createRoot(document.getElementById("root")!)
root.render(<App/>)